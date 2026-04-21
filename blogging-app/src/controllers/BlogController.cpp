#include "controllers/BlogController.h"
#include "services/BlogService.h"
#include <json/json.h>
#include <iostream>

Json::Value BlogController::createErrorResponse(const std::string& message) {
    Json::Value response(Json::objectValue);
    response["success"] = false;
    response["message"] = message;
    return response;
}

Json::Value BlogController::createSuccessResponse(const std::string& message, const Json::Value& data) {
    Json::Value response(Json::objectValue);
    response["success"] = true;
    response["message"] = message;
    response["data"] = data;
    return response;
}

void BlogController::registerRoutes(drogon::HttpAppFramework& app) {
    // CRUD routes
    app.registerHandler("/api/blogs", &createBlog, {drogon::Post});
    app.registerHandler("/api/blogs", &getAllBlogs, {drogon::Get});
    app.registerHandler("/api/blogs/{id}", &getBlogById, {drogon::Get});
    app.registerHandler("/api/blogs/slug/{slug}", &getBlogBySlug, {drogon::Get});
    app.registerHandler("/api/blogs/{id}", &updateBlog, {drogon::Put});
    app.registerHandler("/api/blogs/{id}", &deleteBlog, {drogon::Delete});
    
    // Search routes
    app.registerHandler("/api/search", &searchBlogs, {drogon::Get});
    app.registerHandler("/api/search/tag/{tag}", &searchByTag, {drogon::Get});
    app.registerHandler("/api/search/author/{author}", &searchByAuthor, {drogon::Get});
    app.registerHandler("/api/search/category/{category}", &searchByCategory, {drogon::Get});
    
    // Stats routes
    app.registerHandler("/api/stats", &getBlogStats, {drogon::Get});
    app.registerHandler("/api/tags/popular", &getPopularTags, {drogon::Get});
    
    // Publish routes
    app.registerHandler("/api/blogs/{id}/publish", &publishBlog, {drogon::Post});
    app.registerHandler("/api/blogs/{id}/unpublish", &unpublishBlog, {drogon::Post});
}

void BlogController::createBlog(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        auto json = req->getJsonObject();
        if (!json) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createErrorResponse("Invalid JSON request")
            );
            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }
        
        BlogPost blog;
        
        if ((*json)["title"].isString()) {
            blog.title = (*json)["title"].asString();
        } else {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createErrorResponse("Title is required")
            );
            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }
        
        if ((*json)["subtitle"].isString()) {
            blog.subtitle = (*json)["subtitle"].asString();
        }
        
        if ((*json)["content"].isString()) {
            blog.content = (*json)["content"].asString();
        }
        
        if ((*json)["author"].isString()) {
            blog.author = (*json)["author"].asString();
        }
        
        if ((*json)["category"].isString()) {
            blog.category = (*json)["category"].asString();
        }
        
        if ((*json)["featured_image"].isString()) {
            blog.featured_image = (*json)["featured_image"].asString();
        }
        
        if ((*json)["tags"].isArray()) {
            for (const auto& tag : (*json)["tags"]) {
                if (tag.isString()) {
                    blog.tags.push_back(tag.asString());
                }
            }
        }
        
        BlogService service;
        std::string id = service.createBlog(blog);
        
        if (id.empty()) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createErrorResponse("Failed to create blog")
            );
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
            return;
        }
        
        Json::Value data(Json::objectValue);
        data["id"] = id;
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createSuccessResponse("Blog created successfully", data)
        );
        resp->setStatusCode(drogon::k201Created);
        callback(resp);
        
    } catch (const std::exception& e) {
        std::cerr << "Error creating blog: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::getAllBlogs(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        int page = 1;
        int limit = 10;
        
        auto pageParam = req->getParameter("page");
        auto limitParam = req->getParameter("limit");
        
        if (!pageParam.empty()) {
            page = std::stoi(pageParam);
        }
        if (!limitParam.empty()) {
            limit = std::stoi(limitParam);
        }
        
        BlogService service;
        auto blogs = service.getAllBlogs(page, limit);
        
        Json::Value blogsArray(Json::arrayValue);
        for (const auto& blog : blogs) {
            Json::Value blogJson;
            Json::CharReaderBuilder builder;
            std::string errs;
            std::istringstream s(blog.toJson());
            if (Json::parseFromStream(builder, s, &blogJson, &errs)) {
                blogsArray.append(blogJson);
            }
        }
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createSuccessResponse("Blogs retrieved successfully", blogsArray)
        );
        callback(resp);
        
    } catch (const std::exception& e) {
        std::cerr << "Error getting blogs: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::getBlogById(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                 const std::string& id) {
    try {
        BlogService service;
        auto blog = service.getBlogById(id);
        
        if (blog.title.empty()) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createErrorResponse("Blog not found")
            );
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }
        
        // Increment view count
        service.incrementViewCount(id);
        
        Json::Value blogJson;
        Json::CharReaderBuilder builder;
        std::string errs;
        std::istringstream s(blog.toJson());
        if (Json::parseFromStream(builder, s, &blogJson, &errs)) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createSuccessResponse("Blog retrieved successfully", blogJson)
            );
            callback(resp);
        } else {
            throw std::runtime_error("Failed to parse blog JSON");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error getting blog: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::getBlogBySlug(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                   const std::string& slug) {
    try {
        BlogService service;
        auto blog = service.getBlogBySlug(slug);
        
        if (blog.title.empty()) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createErrorResponse("Blog not found")
            );
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }
        
        // Increment view count
        if (!blog.id.empty()) {
            service.incrementViewCount(blog.id);
        }
        
        Json::Value blogJson;
        Json::CharReaderBuilder builder;
        std::string errs;
        std::istringstream s(blog.toJson());
        if (Json::parseFromStream(builder, s, &blogJson, &errs)) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createSuccessResponse("Blog retrieved successfully", blogJson)
            );
            callback(resp);
        } else {
            throw std::runtime_error("Failed to parse blog JSON");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error getting blog by slug: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::updateBlog(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                const std::string& id) {
    try {
        auto json = req->getJsonObject();
        if (!json) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createErrorResponse("Invalid JSON request")
            );
            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }
        
        BlogPost blog;
        blog.id = id;
        
        if ((*json)["title"].isString()) {
            blog.title = (*json)["title"].asString();
        }
        
        if ((*json)["subtitle"].isString()) {
            blog.subtitle = (*json)["subtitle"].asString();
        }
        
        if ((*json)["content"].isString()) {
            blog.content = (*json)["content"].asString();
        }
        
        if ((*json)["author"].isString()) {
            blog.author = (*json)["author"].asString();
        }
        
        if ((*json)["category"].isString()) {
            blog.category = (*json)["category"].asString();
        }
        
        if ((*json)["featured_image"].isString()) {
            blog.featured_image = (*json)["featured_image"].asString();
        }
        
        BlogService service;
        auto result = service.updateBlog(id, blog);
        
        if (result.empty()) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createErrorResponse("Blog not found or update failed")
            );
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }
        
        Json::Value blogJson;
        Json::CharReaderBuilder builder;
        std::string errs;
        std::istringstream s(result[0].toJson());
        if (Json::parseFromStream(builder, s, &blogJson, &errs)) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createSuccessResponse("Blog updated successfully", blogJson)
            );
            callback(resp);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error updating blog: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::deleteBlog(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                const std::string& id) {
    try {
        BlogService service;
        bool success = service.deleteBlog(id);
        
        if (!success) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createErrorResponse("Blog not found or delete failed")
            );
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createSuccessResponse("Blog deleted successfully", Json::Value(Json::objectValue))
        );
        callback(resp);
        
    } catch (const std::exception& e) {
        std::cerr << "Error deleting blog: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::searchBlogs(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        auto queryParam = req->getParameter("q");
        auto authorParam = req->getParameter("author");
        auto categoryParam = req->getParameter("category");
        auto pageParam = req->getParameter("page");
        auto limitParam = req->getParameter("limit");
        
        int page = 1;
        int limit = 10;
        
        if (!pageParam.empty()) page = std::stoi(pageParam);
        if (!limitParam.empty()) limit = std::stoi(limitParam);
        
        std::vector<std::string> tags;
        // Parse tags from parameter if available
        auto tagsParam = req->getParameter("tags");
        
        BlogService service;
        auto blogs = service.advancedSearch(queryParam, tags, authorParam, categoryParam, page, limit);
        
        Json::Value blogsArray(Json::arrayValue);
        for (const auto& blog : blogs) {
            Json::Value blogJson;
            Json::CharReaderBuilder builder;
            std::string errs;
            std::istringstream s(blog.toJson());
            if (Json::parseFromStream(builder, s, &blogJson, &errs)) {
                blogsArray.append(blogJson);
            }
        }
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createSuccessResponse("Search completed successfully", blogsArray)
        );
        callback(resp);
        
    } catch (const std::exception& e) {
        std::cerr << "Error searching blogs: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::searchByTag(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                 const std::string& tag) {
    try {
        auto pageParam = req->getParameter("page");
        auto limitParam = req->getParameter("limit");
        
        int page = 1;
        int limit = 10;
        
        if (!pageParam.empty()) page = std::stoi(pageParam);
        if (!limitParam.empty()) limit = std::stoi(limitParam);
        
        BlogService service;
        auto blogs = service.searchByTag(tag, page, limit);
        
        Json::Value blogsArray(Json::arrayValue);
        for (const auto& blog : blogs) {
            Json::Value blogJson;
            Json::CharReaderBuilder builder;
            std::string errs;
            std::istringstream s(blog.toJson());
            if (Json::parseFromStream(builder, s, &blogJson, &errs)) {
                blogsArray.append(blogJson);
            }
        }
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createSuccessResponse("Blogs retrieved successfully", blogsArray)
        );
        callback(resp);
        
    } catch (const std::exception& e) {
        std::cerr << "Error searching by tag: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::searchByAuthor(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                    const std::string& author) {
    try {
        auto pageParam = req->getParameter("page");
        auto limitParam = req->getParameter("limit");
        
        int page = 1;
        int limit = 10;
        
        if (!pageParam.empty()) page = std::stoi(pageParam);
        if (!limitParam.empty()) limit = std::stoi(limitParam);
        
        BlogService service;
        auto blogs = service.searchByAuthor(author, page, limit);
        
        Json::Value blogsArray(Json::arrayValue);
        for (const auto& blog : blogs) {
            Json::Value blogJson;
            Json::CharReaderBuilder builder;
            std::string errs;
            std::istringstream s(blog.toJson());
            if (Json::parseFromStream(builder, s, &blogJson, &errs)) {
                blogsArray.append(blogJson);
            }
        }
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createSuccessResponse("Blogs retrieved successfully", blogsArray)
        );
        callback(resp);
        
    } catch (const std::exception& e) {
        std::cerr << "Error searching by author: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::searchByCategory(const drogon::HttpRequestPtr& req,
                                      std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                      const std::string& category) {
    try {
        auto pageParam = req->getParameter("page");
        auto limitParam = req->getParameter("limit");
        
        int page = 1;
        int limit = 10;
        
        if (!pageParam.empty()) page = std::stoi(pageParam);
        if (!limitParam.empty()) limit = std::stoi(limitParam);
        
        BlogService service;
        auto blogs = service.searchByCategory(category, page, limit);
        
        Json::Value blogsArray(Json::arrayValue);
        for (const auto& blog : blogs) {
            Json::Value blogJson;
            Json::CharReaderBuilder builder;
            std::string errs;
            std::istringstream s(blog.toJson());
            if (Json::parseFromStream(builder, s, &blogJson, &errs)) {
                blogsArray.append(blogJson);
            }
        }
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createSuccessResponse("Blogs retrieved successfully", blogsArray)
        );
        callback(resp);
        
    } catch (const std::exception& e) {
        std::cerr << "Error searching by category: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::getBlogStats(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        BlogService service;
        long long total = service.getTotalBlogCount();
        
        Json::Value stats(Json::objectValue);
        stats["total_blogs"] = static_cast<Json::Value::UInt64>(total);
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createSuccessResponse("Stats retrieved successfully", stats)
        );
        callback(resp);
        
    } catch (const std::exception& e) {
        std::cerr << "Error getting stats: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::getPopularTags(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        BlogService service;
        auto tags = service.getPopularTags();
        
        Json::Value tagsArray(Json::arrayValue);
        for (const auto& [tag, count] : tags) {
            Json::Value tagObj(Json::objectValue);
            tagObj["tag"] = tag;
            tagObj["count"] = static_cast<Json::Value::UInt64>(count);
            tagsArray.append(tagObj);
        }
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createSuccessResponse("Tags retrieved successfully", tagsArray)
        );
        callback(resp);
        
    } catch (const std::exception& e) {
        std::cerr << "Error getting tags: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::publishBlog(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                 const std::string& id) {
    try {
        BlogService service;
        bool success = service.publishBlog(id);
        
        if (!success) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createErrorResponse("Blog not found or publish failed")
            );
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createSuccessResponse("Blog published successfully", Json::Value(Json::objectValue))
        );
        callback(resp);
        
    } catch (const std::exception& e) {
        std::cerr << "Error publishing blog: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void BlogController::unpublishBlog(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                   const std::string& id) {
    try {
        BlogService service;
        bool success = service.unpublishBlog(id);
        
        if (!success) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                createErrorResponse("Blog not found or unpublish failed")
            );
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createSuccessResponse("Blog unpublished successfully", Json::Value(Json::objectValue))
        );
        callback(resp);
        
    } catch (const std::exception& e) {
        std::cerr << "Error unpublishing blog: " << e.what() << std::endl;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(
            createErrorResponse("Internal server error")
        );
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

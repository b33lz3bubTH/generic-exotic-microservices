#include "services/BlogService.h"
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

// Static member initialization
mongocxx::client BlogService::mongo_client;
bool BlogService::initialized = false;

BlogService::BlogService() {
    if (!initialized) {
        throw std::runtime_error("BlogService not initialized. Call initialize() first.");
    }
    db = mongo_client["blog_db"];
}

bool BlogService::initialize(const std::string& uri) {
    try {
        static mongocxx::instance inst{};
        mongo_client = mongocxx::client{mongocxx::uri{uri}};
        
        // Test connection
        auto admin = mongo_client.database("admin");
        auto result = admin.run_command(bsoncxx::from_json(R"({"ping": 1})"));
        
        initialized = true;
        std::cout << "MongoDB connection established successfully!" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize MongoDB: " << e.what() << std::endl;
        return false;
    }
}

mongocxx::collection BlogService::getBlogsCollection() {
    auto db = mongo_client["blog_db"];
    return db["blogs"];
}

std::string BlogService::createBlog(const BlogPost& blog) {
    try {
        auto collection = getBlogsCollection();
        
        // Get current timestamp
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        
        BlogPost newBlog = blog;
        newBlog.slug = BlogPost().generateSlug(blog.title);
        newBlog.status = BlogStatus::DRAFT;  // Admin creates blogs as draft
        newBlog.views = 0;
        
        // Format timestamps (simplified - just using current time as string)
        std::stringstream ss;
        std::time_t t = std::time(nullptr);
        ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
        newBlog.created_at = ss.str();
        newBlog.updated_at = ss.str();
        
        auto result = collection.insert_one(newBlog.toBson());
        return result->inserted_id().get_oid().to_string();
    } catch (const std::exception& e) {
        std::cerr << "Error creating blog: " << e.what() << std::endl;
        return "";
    }
}

BlogPost BlogService::getBlogById(const std::string& id) {
    try {
        auto collection = getBlogsCollection();
        bsoncxx::oid oid(id);
        
        auto query = bsoncxx::from_json(R"({"_id": {"$oid": ")" + id + R"("}})");
        auto result = collection.find_one(query);
        
        if (result) {
            return BlogPost::fromBson(result->view());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting blog by ID: " << e.what() << std::endl;
    }
    return BlogPost();
}

BlogPost BlogService::getBlogBySlug(const std::string& slug) {
    try {
        auto collection = getBlogsCollection();
        
        auto query = bsoncxx::from_json(R"({"slug": ")" + slug + R"("})");
        auto result = collection.find_one(query);
        
        if (result) {
            return BlogPost::fromBson(result->view());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting blog by slug: " << e.what() << std::endl;
    }
    return BlogPost();
}

std::vector<BlogPost> BlogService::getAllBlogs(int page, int limit) {
    std::vector<BlogPost> blogs;
    try {
        auto collection = getBlogsCollection();
        
        auto opts = mongocxx::options::find{};
        opts.skip((page - 1) * limit);
        opts.limit(limit);
        opts.sort(bsoncxx::from_json(R"({"created_at": -1})"));
        
        auto cursor = collection.find(bsoncxx::from_json(R"({})"), opts);
        
        for (auto doc : cursor) {
            blogs.push_back(BlogPost::fromBson(doc));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting all blogs: " << e.what() << std::endl;
    }
    return blogs;
}

std::vector<BlogPost> BlogService::updateBlog(const std::string& id, const BlogPost& blog) {
    std::vector<BlogPost> result;
    try {
        auto collection = getBlogsCollection();
        
        auto now = std::chrono::system_clock::now();
        std::stringstream ss;
        std::time_t t = std::time(nullptr);
        ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
        
        BlogPost updatedBlog = blog;
        updatedBlog.updated_at = ss.str();
        
        using bsoncxx::builder::stream::document;
        auto query = document{} << "_id" << bsoncxx::oid(id) << finalize;
        auto update = document{} << "$set" << open_document
            << "title" << updatedBlog.title
            << "subtitle" << updatedBlog.subtitle
            << "content" << updatedBlog.content
            << "category" << updatedBlog.category
            << "author" << updatedBlog.author
            << "featured_image" << updatedBlog.featured_image
            << "updated_at" << updatedBlog.updated_at
            << close_document << finalize;
        
        auto opts = mongocxx::options::find_one_and_update{};
        opts.return_document(mongocxx::options::return_document::k_after);
        
        auto updated = collection.find_one_and_update(query.view(), update.view(), opts);
        
        if (updated) {
            result.push_back(BlogPost::fromBson(updated->view()));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error updating blog: " << e.what() << std::endl;
    }
    return result;
}

bool BlogService::deleteBlog(const std::string& id) {
    try {
        auto collection = getBlogsCollection();
        
        using bsoncxx::builder::stream::document;
        auto query = document{} << "_id" << bsoncxx::oid(id) << finalize;
        
        auto result = collection.delete_one(query.view());
        return result->deleted_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting blog: " << e.what() << std::endl;
        return false;
    }
}

std::vector<BlogPost> BlogService::searchByTitle(const std::string& query, int page, int limit) {
    std::vector<BlogPost> blogs;
    try {
        auto collection = getBlogsCollection();
        
        auto search_query = bsoncxx::from_json(
            R"({"title": {"$regex": ")" + query + R"(", "$options": "i"}})"
        );
        
        auto opts = mongocxx::options::find{};
        opts.skip((page - 1) * limit);
        opts.limit(limit);
        
        auto cursor = collection.find(search_query, opts);
        
        for (auto doc : cursor) {
            blogs.push_back(BlogPost::fromBson(doc));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error searching by title: " << e.what() << std::endl;
    }
    return blogs;
}

std::vector<BlogPost> BlogService::searchByTag(const std::string& tag, int page, int limit) {
    std::vector<BlogPost> blogs;
    try {
        auto collection = getBlogsCollection();
        
        auto search_query = bsoncxx::from_json(
            R"({"tags": ")" + tag + R"("})"
        );
        
        auto opts = mongocxx::options::find{};
        opts.skip((page - 1) * limit);
        opts.limit(limit);
        
        auto cursor = collection.find(search_query, opts);
        
        for (auto doc : cursor) {
            blogs.push_back(BlogPost::fromBson(doc));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error searching by tag: " << e.what() << std::endl;
    }
    return blogs;
}

std::vector<BlogPost> BlogService::searchByAuthor(const std::string& author, int page, int limit) {
    std::vector<BlogPost> blogs;
    try {
        auto collection = getBlogsCollection();
        
        auto search_query = bsoncxx::from_json(
            R"({"author": ")" + author + R"("})"
        );
        
        auto opts = mongocxx::options::find{};
        opts.skip((page - 1) * limit);
        opts.limit(limit);
        
        auto cursor = collection.find(search_query, opts);
        
        for (auto doc : cursor) {
            blogs.push_back(BlogPost::fromBson(doc));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error searching by author: " << e.what() << std::endl;
    }
    return blogs;
}

std::vector<BlogPost> BlogService::searchByCategory(const std::string& category, int page, int limit) {
    std::vector<BlogPost> blogs;
    try {
        auto collection = getBlogsCollection();
        
        auto search_query = bsoncxx::from_json(
            R"({"category": ")" + category + R"("})"
        );
        
        auto opts = mongocxx::options::find{};
        opts.skip((page - 1) * limit);
        opts.limit(limit);
        
        auto cursor = collection.find(search_query, opts);
        
        for (auto doc : cursor) {
            blogs.push_back(BlogPost::fromBson(doc));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error searching by category: " << e.what() << std::endl;
    }
    return blogs;
}

std::vector<BlogPost> BlogService::advancedSearch(
    const std::string& query,
    const std::vector<std::string>& tags,
    const std::string& author,
    const std::string& category,
    int page,
    int limit) {
    
    std::vector<BlogPost> blogs;
    try {
        auto collection = getBlogsCollection();
        
        using bsoncxx::builder::stream::document;
        using bsoncxx::builder::stream::open_array;
        using bsoncxx::builder::stream::close_array;
        
        // Build complex query
        auto builder = document{};
        auto filter = document{};
        
        if (!query.empty()) {
            filter << "$or" << open_array
                << open_document << "title" << bsoncxx::from_json(
                    R"({"$regex": ")" + query + R"(", "$options": "i"})"
                    ) << close_document
                << open_document << "subtitle" << bsoncxx::from_json(
                    R"({"$regex": ")" + query + R"(", "$options": "i"})"
                    ) << close_document
                << open_document << "content" << bsoncxx::from_json(
                    R"({"$regex": ")" + query + R"(", "$options": "i"})"
                    ) << close_document
                << close_array;
        }
        
        if (!author.empty()) {
            filter << "author" << author;
        }
        
        if (!category.empty()) {
            filter << "category" << category;
        }
        
        auto search_query = filter << finalize;
        
        auto opts = mongocxx::options::find{};
        opts.skip((page - 1) * limit);
        opts.limit(limit);
        opts.sort(bsoncxx::from_json(R"({"created_at": -1})"));
        
        auto cursor = collection.find(search_query.view(), opts);
        
        for (auto doc : cursor) {
            blogs.push_back(BlogPost::fromBson(doc));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in advanced search: " << e.what() << std::endl;
    }
    return blogs;
}

long long BlogService::getTotalBlogCount() {
    try {
        auto collection = getBlogsCollection();
        return collection.count_documents(bsoncxx::from_json(R"({})"));
    } catch (const std::exception& e) {
        std::cerr << "Error getting total blog count: " << e.what() << std::endl;
        return 0;
    }
}

long long BlogService::getTotalBlogCountByCategory(const std::string& category) {
    try {
        auto collection = getBlogsCollection();
        auto query = bsoncxx::from_json(R"({"category": ")" + category + R"("})");
        return collection.count_documents(query);
    } catch (const std::exception& e) {
        std::cerr << "Error getting blog count by category: " << e.what() << std::endl;
        return 0;
    }
}

std::vector<std::pair<std::string, long long>> BlogService::getPopularTags() {
    std::vector<std::pair<std::string, long long>> tags;
    // This would require aggregation pipeline - simplified version
    return tags;
}

std::vector<std::pair<std::string, long long>> BlogService::getCategories() {
    std::vector<std::pair<std::string, long long>> categories;
    // This would require aggregation pipeline - simplified version
    return categories;
}

bool BlogService::incrementViewCount(const std::string& id) {
    try {
        auto collection = getBlogsCollection();
        
        using bsoncxx::builder::stream::document;
        auto query = document{} << "_id" << bsoncxx::oid(id) << finalize;
        auto update = document{} << "$inc" << open_document
            << "views" << 1
            << close_document << finalize;
        
        auto result = collection.update_one(query.view(), update.view());
        return result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error incrementing view count: " << e.what() << std::endl;
        return false;
    }
}

bool BlogService::publishBlog(const std::string& id) {
    try {
        auto collection = getBlogsCollection();
        
        using bsoncxx::builder::stream::document;
        auto query = document{} << "_id" << bsoncxx::oid(id) << finalize;
        auto update = document{} << "$set" << open_document
            << "published" << true
            << close_document << finalize;
        
        auto result = collection.update_one(query.view(), update.view());
        return result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error publishing blog: " << e.what() << std::endl;
        return false;
    }
}

bool BlogService::unpublishBlog(const std::string& id) {
    try {
        auto collection = getBlogsCollection();
        
        using bsoncxx::builder::stream::document;
        auto query = document{} << "_id" << bsoncxx::oid(id) << finalize;
        auto update = document{} << "$set" << open_document
            << "published" << false
            << close_document << finalize;
        
        auto result = collection.update_one(query.view(), update.view());
        return result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error unpublishing blog: " << e.what() << std::endl;
        return false;
    }
}

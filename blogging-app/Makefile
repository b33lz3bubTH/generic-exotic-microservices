.PHONY: help build run stop clean logs test dev prod

# Variables
PROJECT_NAME=photo-sharing-app
BUILD_DIR=build
DOCKERFILE=Dockerfile
COMPOSE_FILE=docker-compose.yml

help:
	@echo "═══════════════════════════════════════════════════════════"
	@echo "      📸 PHOTO SHARING SERVICE - MAKEFILE COMMANDS"
	@echo "═══════════════════════════════════════════════════════════"
	@echo ""
	@echo "📦 Build Commands:"
	@echo "  make build          - Build the project with CMake"
	@echo "  make docker-build   - Build Docker image"
	@echo ""
	@echo "🚀 Run Commands:"
	@echo "  make run            - Run the application locally"
	@echo "  make docker-run     - Run using Docker Compose"
	@echo "  make start          - Start Docker services"
	@echo "  make dev            - Start in development mode"
	@echo "  make prod           - Start in production mode"
	@echo ""
	@echo "🛑 Stop Commands:"
	@echo "  make stop           - Stop Docker services"
	@echo "  make down           - Stop and remove containers"
	@echo ""
	@echo "🧹 Clean Commands:"
	@echo "  make clean          - Clean build directory"
	@echo "  make clean-docker   - Remove Docker containers and images"
	@echo "  make clean-all      - Full cleanup"
	@echo ""
	@echo "📊 Monitoring:"
	@echo "  make logs           - View combined logs"
	@echo "  make logs-app       - View app logs"
	@echo "  make logs-mongo     - View MongoDB logs"
	@echo "  make ps             - Show running containers"
	@echo ""
	@echo "🧪 Testing:"
	@echo "  make test           - Run API tests"
	@echo "  make health         - Check service health"
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"

# Build targets
build:
	@echo "🔨 Building project..."
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && make -j$$(nproc)
	@echo "✅ Build complete!"

docker-build:
	@echo "🐳 Building Docker image..."
	docker build -t $(PROJECT_NAME):latest -f $(DOCKERFILE) .
	@echo "✅ Docker image built!"

# Run targets
run: build
	@echo "🚀 Running application..."
	./$(BUILD_DIR)/$(PROJECT_NAME)

docker-run: docker-build
	@echo "🐳 Running with Docker Compose..."
	docker-compose -f $(COMPOSE_FILE) up --build

start:
	@echo "🚀 Starting services..."
	docker-compose -f $(COMPOSE_FILE) up -d
	@echo "✅ Services started!"
	@echo "📱 App: http://localhost:8080"
	@echo "🗄️  MongoDB: localhost:27017"
	@echo "📊 Mongo Express: http://localhost:8081"

dev:
	@echo "🔧 Starting development environment..."
	docker-compose -f $(COMPOSE_FILE) -f docker-compose.dev.yml up --build

prod:
	@echo "🚀 Starting production environment..."
	docker-compose -f $(COMPOSE_FILE) up -d

# Stop targets
stop:
	@echo "🛑 Stopping services..."
	docker-compose -f $(COMPOSE_FILE) stop
	@echo "✅ Services stopped!"

down:
	@echo "🛑 Stopping and removing services..."
	docker-compose -f $(COMPOSE_FILE) down
	@echo "✅ Services removed!"

# Clean targets
clean:
	@echo "🧹 Cleaning build directory..."
	rm -rf $(BUILD_DIR)
	@echo "✅ Cleaned!"

clean-docker:
	@echo "🐳 Removing Docker containers and images..."
	docker-compose -f $(COMPOSE_FILE) down -v
	docker rmi $(PROJECT_NAME):latest 2>/dev/null || true
	@echo "✅ Docker cleaned!"

clean-all: clean clean-docker
	@echo "✅ Full cleanup complete!"

# Logging targets
logs:
	@echo "📋 Combined logs:"
	docker-compose -f $(COMPOSE_FILE) logs -f

logs-app:
	@echo "📋 Application logs:"
	docker-compose -f $(COMPOSE_FILE) logs -f app

logs-mongo:
	@echo "📋 MongoDB logs:"
	docker-compose -f $(COMPOSE_FILE) logs -f mongo

ps:
	@echo "🐳 Running containers:"
	docker-compose -f $(COMPOSE_FILE) ps

# Testing targets
test:
	@echo "🧪 Running API tests..."
	@if [ -f test-api.sh ]; then \
		bash test-api.sh; \
	else \
		echo "❌ test-api.sh not found!"; \
	fi

health:
	@echo "🏥 Checking service health..."
	@echo "⏳ Waiting for services to be ready..."
	@sleep 2
	@curl -s http://localhost:8080/api/stats | jq '.' && echo "✅ API is healthy!" || echo "❌ API is down!"

# Utility targets
restart: stop start
	@echo "✅ Services restarted!"

install-deps:
	@echo "📦 Installing dependencies..."
	@echo "Note: Run this on the host system, not in Docker"
	@echo "Ubuntu/Debian: apt-get install -y cmake g++ libssl-dev libbsoncxx-dev libmongocxx-dev jsoncpp libjsoncpp-dev"
	@echo "macOS: brew install cmake openssl mongodb-community jsoncpp"

# Development helpers
format:
	@echo "📝 Formatting code..."
	clang-format -i src/**/*.cpp include/**/*.h 2>/dev/null || echo "clang-format not found"

lint:
	@echo "🔍 Linting code..."
	clang-tidy src/**/*.cpp -- -I./include 2>/dev/null || echo "clang-tidy not found"

shell-app:
	@echo "🐚 Opening app container shell..."
	docker-compose -f $(COMPOSE_FILE) exec app bash

shell-mongo:
	@echo "🐚 Opening MongoDB container shell..."
	docker-compose -f $(COMPOSE_FILE) exec mongo mongosh

# Documentation
docs:
	@echo "📚 Available documentation:"
	@echo "  - README.md        - Project overview and setup"
	@echo "  - API-DOCS.md      - Complete API documentation"
	@echo "  - test-api.sh      - Testing script with examples"

version:
	@echo "📦 Project: $(PROJECT_NAME)"
	@echo "🔨 Build system: CMake"
	@echo "🌐 Framework: Drogon"
	@echo "🗄️  Database: MongoDB"
	@echo "🐳 Containerization: Docker"

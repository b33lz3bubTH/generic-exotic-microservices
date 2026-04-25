.PHONY: build run test docker-up docker-down clean

BUILD_DIR=build
COMPOSE_FILE=docker-compose.yml

build:
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) -j$$(nproc)

run: build
	PHOTO_ADMIN_KEY=$${PHOTO_ADMIN_KEY:-change-me-admin-key} ./$(BUILD_DIR)/album_sharing_service

test:
	bash e2e/run.sh

docker-up:
	docker compose -f $(COMPOSE_FILE) up --build -d

docker-down:
	docker compose -f $(COMPOSE_FILE) down -v

clean:
	rm -rf $(BUILD_DIR)

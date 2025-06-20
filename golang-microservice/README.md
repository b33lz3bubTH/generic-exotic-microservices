# ContactUs Microservice

A production-grade, SOLID-principled Go microservice for handling Contact Us queries, using SQLite and GORM. Designed for extensibility and industry best practices.

## Features
- Clean, layered architecture (models, repository, service, controller)
- SOLID principles and dependency injection
- SQLite database (GORM ORM)
- Async-ready service layer
- Gin HTTP framework
- Fuzzy search and pagination on GET /contactus
- UUID for entries, JSON field for extras
- Logging with logrus
- Easily extensible for Redis, OpenTelemetry, etc.

## Endpoints
- `POST /contactus` — Create a new contact entry, returns UUID
- `GET /contactus` — Fuzzy search by any field, paginated, ordered by latest

## Project Structure
```
internal/
  models/        # GORM models, DBI abstraction
  repository/    # Repository interfaces and implementations
  service/       # Business logic, async-ready
  controller/    # Gin handlers, validation
  db/            # DB initialization
main.go          # App entrypoint, DI, router
```

## Running
```sh
go run main.go
```

## Extensibility
- Add Redis, OpenTelemetry, or other infra by extending the repository/service layers.
- All layers are interface-driven for easy testing and replacement.

---
**By design, this codebase is ready for production and future scaling.** 
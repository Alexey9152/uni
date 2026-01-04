# To-Do List API (учебный проект)

## Кратко
Проект реализует REST API для управления задачами (To‑Do List) с CRUD‑эндпоинтами `/tasks` и обменом данными в JSON.

Дополнительно проект развёрнут как набор сервисов в Docker Compose и включает инфраструктуру: API Gateway, БД, кэш, очередь сообщений, сбор метрик (Prometheus) и визуализацию (Grafana), а также Swagger UI для документации API.

## Соответствие требованиям
### Минимум (обязательная часть)
- Реализован REST CRUD API для ресурса **Task**.
- Используются стандартные HTTP методы: `GET`, `POST`, `PUT`, `DELETE` (а также `PATCH` для частичного обновления статуса).
- Корректные HTTP коды: `200`, `201`, `404`.
- Формат обмена данными: JSON.

### Что добавлено сверх минимума
- Работа с БД: PostgreSQL.
- API Gateway: Spring Cloud Gateway (единая точка входа).
- Ограничение трафика: rate limiting в Gateway (Redis).
- Кэш: Redis.
- Очередь сообщений: RabbitMQ + отдельный worker.
- Метрики и мониторинг: Prometheus + Grafana.
- Базовая отказоустойчивость: retry/circuit breaker/fallback в Gateway.
- Автоматизация развёртывания окружения: `docker compose up` поднимает весь стек локально.

## Модель данных
### Task
Поля задачи:
- `id` — идентификатор
- `title` — название
- `description` — описание
- `status` — статус (`todo`, `in_progress`, `done`)

## Эндпоинты API
Базовый URL (через Gateway): `http://localhost:8080`

- `GET /tasks` — получить список задач.
- `POST /tasks` — создать задачу.
- `GET /tasks/{id}` — получить задачу по ID.
- `PUT /tasks/{id}` — полностью обновить задачу.
- `PATCH /tasks/{id}` — частично обновить задачу (например, статус).
- `DELETE /tasks/{id}` — удалить задачу.

## Контракты API (JSON, коды, структуры)

### Общие правила
- Все запросы/ответы используют JSON и заголовок `Content-Type: application/json` (для запросов с телом).
- При успешных операциях используются стандартные коды `200 OK`, `201 Created`, а при отсутствии ресурса — `404 Not Found`.

### Сущность Task (JSON)
Полная структура задачи:
```json
{
  "id": 1,
  "title": "Купить молоко",
  "description": "Обязательно 3.2% жирности",
  "status": "todo"
}
```

Ограничения:
- `status` ∈ `todo | in_progress | done`.

---

### GET /tasks — список задач
**Запрос:** без тела.

**Ответ (200 OK):**
```json
[
  { "id": 1, "title": "Купить молоко", "description": "Обязательно 3.2%", "status": "todo" },
  { "id": 2, "title": "Запустить API", "description": "…", "status": "in_progress" }
]
```

**Коды:**
- `200 OK` — список возвращён.

---

### POST /tasks — создать задачу
**Запрос (JSON):**
```json
{
  "title": "Купить молоко",
  "description": "Обязательно 3.2% жирности",
  "status": "todo"
}
```

**Ответ (201 Created, JSON):**
```json
{
  "id": 1,
  "title": "Купить молоко",
  "description": "Обязательно 3.2% жирности",
  "status": "todo"
}
```

**Коды:**
- `201 Created` — задача создана.
- `400 Bad Request` — невалидный JSON/поля (например, неизвестный `status`).

---

### GET /tasks/{id} — получить задачу
**Запрос:** без тела.

**Ответ (200 OK, JSON):** возвращается объект Task.

**Коды:**
- `200 OK` — задача найдена.
- `404 Not Found` — задачи с таким `id` нет.

---

### PUT /tasks/{id} — полное обновление
**Запрос (JSON):**
```json
{
  "title": "Купить хлеб",
  "description": "Бородинский",
  "status": "in_progress"
}
```

**Ответ (200 OK, JSON):** обновлённый Task.

**Коды:**
- `200 OK` — обновлено.
- `404 Not Found` — задачи нет.
- `400 Bad Request` — невалидные данные.

---

### PATCH /tasks/{id} — частичное обновление (например, статус)
**Запрос (JSON):**
```json
{ "status": "done" }
```

**Ответ (200 OK, JSON):** обновлённый Task.

**Коды:**
- `200 OK` — обновлено.
- `404 Not Found` — задачи нет.
- `400 Bad Request` — невалидные данные.

---

### DELETE /tasks/{id} — удалить задачу
**Запрос:** без тела.

**Ответ:** без тела или JSON (в зависимости от реализации).

**Коды:**
- `200 OK` или `204 No Content` — удалено.
- `404 Not Found` — задачи нет.

## Примеры запросов (Windows cmd)
Проверка списка:
```bat
curl -i http://localhost:8080/tasks
```

Создание задачи:
```bat
curl -i -X POST http://localhost:8080/tasks ^
  -H "Content-Type: application/json" ^
  -d "{\"title\":\"Купить молоко\",\"description\":\"Обязательно 3.2%\",\"status\":\"todo\"}"
```

Получить задачу по ID:
```bat
curl -i http://localhost:8080/tasks/1
```

Полное обновление (PUT):
```bat
curl -i -X PUT http://localhost:8080/tasks/1 ^
  -H "Content-Type: application/json" ^
  -d "{\"title\":\"Купить хлеб\",\"description\":\"Бородинский\",\"status\":\"in_progress\"}"
```

Частичное обновление статуса (PATCH):
```bat
curl -i -X PATCH http://localhost:8080/tasks/1 ^
  -H "Content-Type: application/json" ^
  -d "{\"status\":\"done\"}"
```

Удаление:
```bat
curl -i -X DELETE http://localhost:8080/tasks/1
```

## Swagger UI
Swagger UI (через Gateway):
- `http://localhost:8080/swagger-ui/index.html`

OpenAPI JSON:
- `http://localhost:8080/v3/api-docs`

## Мониторинг
### Prometheus
Prometheus UI (через Gateway):
- `http://localhost:8080/prometheus/`

Проверка таргетов:
- `http://localhost:8080/prometheus/targets`

### Grafana
Grafana UI (через Gateway):
- `http://localhost:8080/grafana/`

В Grafana добавлен Prometheus datasource, который указывает на Prometheus внутри docker-сети.

## Как запустить проект
Из корня проекта:
```bat
docker compose up -d --build
```

Полезные команды:
```bat
docker ps

docker logs -f todo-gateway

docker logs -f todo-api

docker logs -f todo-prometheus

docker logs -f todo-grafana
```

## Состав контейнеров (Docker Compose)
- `todo-gateway` — входная точка (порт 8080 наружу).
- `todo-api` — CRUD API.
- `todo-postgres` — PostgreSQL.
- `todo-redis` — Redis.
- `todo-rabbitmq` — RabbitMQ.
- `todo-events-worker` — обработчик событий.
- `todo-prometheus` — сбор метрик.
- `todo-grafana` — визуализация.

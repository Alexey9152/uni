# API Documentation - Библиотечная система

## 📌 Базовая информация

- **Хост**: `localhost:8080`
- **Аутентификация**: Spring Security (Session-based)
- **Content-Type**: `application/json`
- **Кодировка**: UTF-8

---

## 🔐 Аутентификация

### Регистрация
```
POST /auth/register
Content-Type: application/x-www-form-urlencoded

Parameters:
- username: string (уникальное имя)
- password: string (любая сложность)
- fullName: string (ФИО)
- role: string (LIBRARIAN или READER)

Response: Redirect to /login
```

### Вход
```
POST /login
Content-Type: application/x-www-form-urlencoded

Parameters:
- username: string
- password: string

Response: Redirect to / (автоматический выбор панели)
```

### Выход
```
GET /logout

Response: Redirect to /login
```

---

## 📚 API Библиотекаря

### 1. Получить все книги
```
GET /api/librarian/books

Response (200):
[
  {
    "id": "1",
    "title": "Война и мир",
    "authors": "Лев Толстой",
    "year": 1869,
    "category": "Роман",
    "price": 450.00,
    "total": 5,
    "available": 3
  },
  ...
]

Ошибки:
403 - Нет доступа (не библиотекарь)
```

### 2. Добавить новую книгу
```
POST /api/librarian/books
Content-Type: application/json

Request Body:
{
  "id": "10",
  "title": "Новая книга",
  "authors": "Автор1, Автор2",
  "year": 2024,
  "category": "Фантастика",
  "price": 599.99,
  "total": 3,
  "available": 2
}

Response (200):
"Книга добавлена успешно"

Ошибки:
400 - Некорректные данные
403 - Нет доступа
```

### 3. Изменить цену книги
```
PUT /api/librarian/books/{bookId}/price?price={newPrice}

Parameters:
- bookId: string (ID книги)
- price: double (новая цена)

Response (200):
"Цена обновлена"

Примеры:
PUT /api/librarian/books/1/price?price=599.99
PUT /api/librarian/books/5/price?price=1000.00

Ошибки:
400 - Некорректная цена
403 - Нет доступа
404 - Книга не найдена
```

### 4. Выдать книгу читателю
```
POST /api/librarian/books/{bookId}/issue?readerUsername={username}

Parameters:
- bookId: string (ID книги)
- readerUsername: string (username читателя)

Response (200):
"Книга выдана успешно"

Примеры:
POST /api/librarian/books/1/issue?readerUsername=reader1
POST /api/librarian/books/5/issue?readerUsername=petrov

Ошибки:
400 - Нет доступных экземпляров
403 - Нет доступа
404 - Книга или читатель не найдены
```

### 5. Получить всех читателей
```
GET /api/librarian/readers

Response (200):
[
  {
    "username": "reader1",
    "fullName": "Петр Читатель",
    "role": "READER",
    "borrowedBookIds": ["1", "3", "5"]
  },
  ...
]

Ошибки:
403 - Нет доступа
```

### 6. Получить информацию читателя
```
GET /api/librarian/readers/{username}

Parameters:
- username: string (username читателя)

Response (200):
{
  "username": "reader1",
  "fullName": "Петр Читатель",
  "role": "READER",
  "borrowedBookIds": ["1", "3"]
}

Ошибки:
403 - Нет доступа
404 - Читатель не найден
```

---

## 📖 API Читателя

### 1. Получить все доступные книги
```
GET /api/reader/books

Response (200):
[
  {
    "id": "1",
    "title": "Война и мир",
    "authors": "Лев Толстой",
    "year": 1869,
    "category": "Роман",
    "price": 450.00,
    "total": 5,
    "available": 3
  },
  ...
]

Ошибки:
403 - Нет доступа
```

### 2. Поиск по автору
```
GET /api/reader/books/search/author?author={query}

Parameters:
- author: string (имя автора или часть имени)

Response (200):
[
  {
    "id": "1",
    "title": "Война и мир",
    "authors": "Лев Толстой",
    ...
  },
  ...
]

Примеры:
GET /api/reader/books/search/author?author=Толстой
GET /api/reader/books/search/author?author=Достоев
GET /api/reader/books/search/author?author=Булгаков

Ошибки:
403 - Нет доступа
```

### 3. Поиск по году издания
```
GET /api/reader/books/search/year?year={year}

Parameters:
- year: integer (год издания)

Response (200):
[
  {
    "id": "1",
    "title": "Война и мир",
    "year": 1869,
    ...
  },
  ...
]

Примеры:
GET /api/reader/books/search/year?year=1869
GET /api/reader/books/search/year?year=1966

Ошибки:
403 - Нет доступа
```

### 4. Поиск по категории
```
GET /api/reader/books/search/category?category={query}

Parameters:
- category: string (название или часть названия категории)

Response (200):
[
  {
    "id": "1",
    "title": "Война и мир",
    "category": "Роман",
    ...
  },
  ...
]

Примеры:
GET /api/reader/books/search/category?category=Роман
GET /api/reader/books/search/category?category=Фантастика
GET /api/reader/books/search/category?category=Детектив

Ошибки:
403 - Нет доступа
```

### 5. Получить свой аккаунт
```
GET /api/reader/account

Response (200):
{
  "username": "reader1",
  "fullName": "Петр Читатель",
  "role": "READER",
  "borrowedBookIds": ["1", "3", "5"]
}

Ошибки:
403 - Нет доступа
404 - Пользователь не найден
```

### 6. Получить список выданных книг
```
GET /api/reader/borrowed-books

Response (200):
[
  {
    "id": "1",
    "title": "Война и мир",
    "authors": "Лев Толстой",
    "year": 1869,
    "category": "Роман",
    "price": 450.00,
    "total": 5,
    "available": 3
  },
  ...
]

Ошибки:
403 - Нет доступа
```

---

## 📋 Примеры cURL команд

### Вход в систему
```bash
curl -c cookies.txt -X POST http://localhost:8080/login \
  -d "username=librarian1&password=pass123"
```

### Получить все книги (с сохраненной сессией)
```bash
curl -b cookies.txt http://localhost:8080/api/librarian/books
```

### Добавить книгу
```bash
curl -b cookies.txt -X POST http://localhost:8080/api/librarian/books \
  -H "Content-Type: application/json" \
  -d '{
    "id": "10",
    "title": "Новая книга",
    "authors": "Автор",
    "year": 2024,
    "category": "Фантастика",
    "price": 599.99,
    "total": 5,
    "available": 3
  }'
```

### Изменить цену
```bash
curl -b cookies.txt -X PUT \
  "http://localhost:8080/api/librarian/books/1/price?price=799.99"
```

### Выдать книгу
```bash
curl -b cookies.txt -X POST \
  "http://localhost:8080/api/librarian/books/1/issue?readerUsername=reader1"
```

### Поиск по автору
```bash
curl -b cookies.txt \
  "http://localhost:8080/api/reader/books/search/author?author=Толстой"
```

### Поиск по году
```bash
curl -b cookies.txt \
  "http://localhost:8080/api/reader/books/search/year?year=1869"
```

### Поиск по категории
```bash
curl -b cookies.txt \
  "http://localhost:8080/api/reader/books/search/category?category=Роман"
```

---

## 🔄 Коды ответов

| Код | Описание |
|-----|---------|
| 200 | OK - Запрос выполнен успешно |
| 400 | Bad Request - Некорректные данные |
| 403 | Forbidden - Нет доступа (не авторизирован или недостаточно прав) |
| 404 | Not Found - Ресурс не найден |
| 500 | Internal Server Error - Ошибка сервера |

---

## 📝 Основные объекты

### Book
```json
{
  "id": "1",
  "title": "Война и мир",
  "authors": "Лев Толстой",
  "year": 1869,
  "category": "Роман",
  "price": 450.00,
  "total": 5,
  "available": 3
}
```

### User
```json
{
  "username": "reader1",
  "fullName": "Петр Читатель",
  "role": "READER",
  "borrowedBookIds": ["1", "3", "5"]
}
```

### Error Response
```json
{
  "error": "Описание ошибки",
  "status": 400
}
```

---

## 🔐 Права доступа

### Публичные endpoint'ы (без аутентификации)
- `POST /auth/register` - Регистрация
- `POST /login` - Вход
- `GET /login` - Страница входа
- `GET /register` - Страница регистрации

### Endpoints для библиотекаря
- `GET /api/librarian/books` - просмотр
- `POST /api/librarian/books` - добавление
- `PUT /api/librarian/books/{id}/price` - переоценка
- `POST /api/librarian/books/{bookId}/issue` - выдача
- `GET /api/librarian/readers` - просмотр читателей

### Endpoints для читателя
- `GET /api/reader/books` - просмотр каталога
- `GET /api/reader/books/search/*` - поиск
- `GET /api/reader/account` - информация о себе
- `GET /api/reader/borrowed-books` - выданные книги

---

## ⚠️ Важные замечания

1. **Аутентификация**: Все API требуют валидной сессии (кроме регистрации/входа)
2. **Авторизация**: Запросы проверяются по ролям пользователя
3. **XML**: Все изменения автоматически сохраняются в XML файлы
4. **Кодировка**: UTF-8 для всех русских текстов
5. **Цены**: Используйте формат decimal (например, 450.00)

---

## 🧪 Тестирование

Используйте Postman или аналогичный инструмент для тестирования API:

1. Создайте окружение с переменной `base_url = http://localhost:8080`
2. Используйте authentication type "Cookie" для сохранения сессии
3. Сначала выполните POST /login, затем другие запросы

---

**Документация актуальна для версии 1.0.0**

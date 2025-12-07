# Комплексное руководство для разработчика

## 1️⃣ ПЕРВЫЕ ШАГИ

### Создание структуры проекта Maven:

```bash
mvn archetype:generate \
  -DgroupId=com.library \
  -DartifactId=library-web-app \
  -DarchetypeArtifactId=maven-archetype-quickstart \
  -DinteractiveMode=false

cd library-web-app
```

### Копирование файлов:

1. Скопируйте **pom.xml** в корень проекта
2. Создайте папки:
   ```
   src/main/java/com/library/
   src/main/java/com/library/config/
   src/main/java/com/library/controller/
   src/main/java/com/library/model/
   src/main/java/com/library/service/
   src/main/resources/templates/
   src/main/resources/data/
   ```

3. Скопируйте все файлы Java в соответствующие папки:
   - **LibraryApplication.java** → `src/main/java/com/library/`
   - **SecurityConfig.java** → `src/main/java/com/library/config/`
   - **ViewController.java, AuthController.java, LibrarianController.java, ReaderController.java** → `src/main/java/com/library/controller/`
   - **Book.java, User.java, UserRole.java** → `src/main/java/com/library/model/`
   - **BookService.java, UserService.java, XmlService.java, EncryptionService.java** → `src/main/java/com/library/service/`

4. Скопируйте HTML файлы в `src/main/resources/templates/`

5. Скопируйте XML файлы в `src/main/resources/data/`

6. Скопируйте **application.properties** в `src/main/resources/`

## 2️⃣ СБОРКА И ЗАПУСК

### Сборка:
```bash
mvn clean install
```

### Запуск:
```bash
mvn spring-boot:run
```

### Или через JAR:
```bash
java -jar target/library-web-app-1.0.0.jar
```

## 3️⃣ ПЕРВЫЙ ЗАПУСК

1. Откройте `http://localhost:8080/`
2. Перейдите на страницу регистрации (`/register`)
3. Создайте тестового библиотекаря:
   - Username: `librarian1`
   - Password: `pass123`
   - Full Name: `Иван Библиотекарь`
   - Role: `LIBRARIAN`

4. Создайте тестового читателя:
   - Username: `reader1`
   - Password: `pass456`
   - Full Name: `Петр Читатель`
   - Role: `READER`

5. Вернитесь на логин (`/login`) и попробуйте войти

## 4️⃣ ТЕСТИРОВАНИЕ ФУНКЦИОНАЛА

### Панель библиотекаря:
- [ ] Добавить новую книгу
- [ ] Просмотреть таблицу всех книг
- [ ] Изменить цену книги
- [ ] Выдать книгу читателю (вводим reader1)
- [ ] Просмотреть список читателей

### Панель читателя:
- [ ] Просмотреть каталог
- [ ] Поиск по автору (попробуйте "Толстой")
- [ ] Поиск по году (попробуйте "1869")
- [ ] Поиск по категории (попробуйте "Роман")
- [ ] Посмотреть мои выданные книги
- [ ] Посмотреть информацию аккаунта

## 5️⃣ ПРОВЕРКА XML

### После добавления книги проверьте файл:
```bash
# Windows
type src\main\resources\data\library.xml

# Linux/Mac
cat src/main/resources/data/library.xml
```

Должна быть новая книга в XML!

### После регистрации проверьте:
```bash
# Windows
type src\main\resources\data\users.xml

# Linux/Mac
cat src/main/resources/data/users.xml
```

Должны быть новые пользователи с хэшированными паролями!

## 6️⃣ ОСНОВНЫЕ КОМПОНЕНТЫ И ИХ ФУНКЦИИ

### XmlService.java
```
Это ГЛАВНЫЙ сервис работы с данными!

✅ loadBooks() - загружает книги из library.xml
✅ saveBooks() - сохраняет книги в library.xml
✅ loadUsers() - загружает пользователей из users.xml
✅ saveUsers() - сохраняет пользователей в users.xml
✅ validateXml() - проверяет валидность по XSD

⚠️ КАЖДОЕ ИЗМЕНЕНИЕ ДАННЫХ АВТОМАТИЧЕСКИ СОХРАНЯЕТСЯ!
```

### BookService.java
```
Бизнес-логика для работы с книгами:
✅ Добавление, обновление, удаление
✅ Поиск по различным полям
✅ Автоматический вызов saveBooks()
```

### UserService.java
```
Бизнес-логика для пользователей:
✅ Регистрация
✅ Аутентификация (проверка пароля)
✅ Управление выданными книгами
✅ Автоматический вызов saveUsers()
```

### EncryptionService.java
```
Безопасность паролей:
✅ hashPassword() - хэширует пароль с солью
✅ verifyPassword() - проверяет пароль
✅ Использует SHA-256 + 16-байтная соль
```

## 7️⃣ АРХИТЕКТУРА СИСТЕМЫ

```
                    HTTP Request
                        ↓
    ┌─────────────────────────────────────┐
    │         Spring Security Filter      │
    │  (Проверка аутентификации/авторизации)
    └────────────────┬────────────────────┘
                     ↓
    ┌─────────────────────────────────────┐
    │    ViewController / AuthController   │ ← Возвращает HTML
    │    LibrarianController             │ ← REST API JSON
    │    ReaderController                │ ← REST API JSON
    └────────────────┬────────────────────┘
                     ↓
    ┌─────────────────────────────────────┐
    │         BookService                 │
    │         UserService                 │
    │    (Бизнес-логика)                 │
    └────────────────┬────────────────────┘
                     ↓
    ┌─────────────────────────────────────┐
    │         XmlService                  │ ← DOM Parser
    │  (Загрузка/Сохранение XML)          │ ← XSD Validation
    │    EncryptionService                │ ← SHA-256 + соль
    └────────────────┬────────────────────┘
                     ↓
    ┌─────────────────────────────────────┐
    │      library.xml / users.xml        │
    │      (Данные на диске)              │
    └─────────────────────────────────────┘
```

## 8️⃣ ВАЖНЫЕ МОМЕНТЫ

### 🔒 Безопасность
- Пароли НИКОГДА не сохраняются в открытом виде
- Используется SHA-256 с уникальной солью для каждого пароля
- Spring Security проверяет права доступа на каждый endpoint

### 💾 Сохранение данных
- XML файлы сохраняются в `src/main/resources/data/`
- При каждом изменении данные автоматически пишутся на диск
- XML валидируется по XSD схемам

### 🔄 Поток данных
1. Пользователь делает действие на HTML странице
2. JavaScript отправляет AJAX запрос
3. Controller обрабатывает запрос
4. Service вызывает бизнес-логику
5. XmlService сохраняет/загружает XML
6. Данные возвращаются пользователю

## 9️⃣ ОТЛАДКА

### Проверить консоль:
```
При запуске должны быть строки:
- "Tomcat started on port(s): 8080"
- Никаких ERROR в логах
```

### Проверить доступ:
```
curl http://localhost:8080/login
```

### Проверить XML:
```
Откройте файлы в src/main/resources/data/
library.xml - должен содержать книги
users.xml - должен содержать пользователей
```

### Если не работает:
1. Проверьте версию Java: `java -version` (должна быть 17+)
2. Проверьте Maven: `mvn -version`
3. Удалите .m2 кэш: `rm -rf ~/.m2/repository` (Linux/Mac)
4. Пересоберите: `mvn clean install`

## 🔟 ДОПОЛНИТЕЛЬНЫЕ УЛУЧШЕНИЯ (ОПЦИОНАЛЬНО)

### Добавить инициализацию тестовых данных:
В `BookService.java` конструкторе добавьте:
```java
// Инициализация при пустом файле
if (this.books.isEmpty()) {
    this.books.add(new Book("test1", "Тестовая книга", "Автор", 2024, "Тест", 99.99, 5, 3));
    saveBooks();
}
```

### Добавить пагинацию:
В `LibrarianController.java` измените:
```java
@GetMapping("/books")
public ResponseEntity<List<Book>> getAllBooks(
        @RequestParam(defaultValue = "0") int page,
        @RequestParam(defaultValue = "10") int size,
        Authentication auth) {
    // Реализуйте пагинацию
}
```

### Добавить логирование:
```java
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

private static final Logger logger = LoggerFactory.getLogger(XmlService.class);

logger.info("Loading books from XML");
logger.debug("Found {} books", books.size());
```

## 📖 Полезные ссылки

- Spring Boot: https://spring.io/projects/spring-boot
- Spring Security: https://spring.io/projects/spring-security
- Thymeleaf: https://www.thymeleaf.org/
- Java XML DOM: https://docs.oracle.com/en/java/javase/17/docs/api/java.xml/org/w3c/dom/package-summary.html

---

**Успехов в разработке! 🚀**

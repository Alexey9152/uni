
# Создам для вас структуру всех необходимых файлов проекта
# с кратким описанием каждого

project_structure = """
library-web-app/
├── pom.xml                          # Maven конфигурация
├── src/main/java/com/library/
│   ├── LibraryApplication.java      # Main класс Spring Boot
│   ├── config/
│   │   ├── SecurityConfig.java      # Конфигурация безопасности
│   │   └── WebConfig.java           # Веб конфигурация
│   ├── controller/
│   │   ├── AuthController.java      # Регистрация/логирование
│   │   ├── LibrarianController.java # API для библиотекаря
│   │   ├── ReaderController.java    # API для читателя
│   │   └── ViewController.java      # Возврат HTML-страниц
│   ├── model/
│   │   ├── Book.java                # Модель книги
│   │   ├── User.java                # Модель пользователя
│   │   └── UserRole.java            # Перечисление ролей
│   ├── service/
│   │   ├── BookService.java         # Бизнес-логика для книг
│   │   ├── UserService.java         # Бизнес-логика для пользователей
│   │   ├── XmlService.java          # XML сохранение/загрузка
│   │   └── EncryptionService.java   # Шифрование паролей
│   └── dto/
│       ├── BookDTO.java             # DTO для книг
│       └── UserDTO.java             # DTO для пользователей
├── src/main/resources/
│   ├── application.properties       # Свойства приложения
│   ├── templates/
│   │   ├── login.html               # Страница логирования
│   │   ├── register.html            # Страница регистрации
│   │   ├── librarian-dashboard.html # Панель библиотекаря
│   │   └── reader-dashboard.html    # Панель читателя
│   └── data/
│       ├── library.xml              # Книги
│       ├── library.xsd              # Схема XML для книг
│       ├── users.xml                # Пользователи
│       └── users.xsd               # Схема XML для пользователей
└── README.md
"""

print(project_structure)
print("\n" + "="*70)
print("ВСЕГО ФАЙЛОВ: ~25 файлов для полнофункциональной системы")
print("="*70)

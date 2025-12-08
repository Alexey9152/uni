package com.library.controller;

import com.library.model.Book;
import com.library.model.User;
import com.library.model.UserRole;
import com.library.service.BookOrderService;
import com.library.service.BookService;
import com.library.service.UserService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Optional;

@RestController
@RequestMapping("/api/librarian")
public class LibrarianController {

    @Autowired
    private BookService bookService;

    @Autowired
    private UserService userService;

    @Autowired
    private BookOrderService bookOrderService;

    /**
     * Получить все книги
     */
    @GetMapping("/books")
    public ResponseEntity<List<Book>> getAllBooks(Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }
        return ResponseEntity.ok(bookService.getAllBooks());
    }

    /**
     * Добавить новую книгу
     */
    @PostMapping("/books")
    public ResponseEntity<String> addBook(@RequestBody Book book, Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }

        try {
            // Валидация данных книги
            String validationError = validateBook(book);
            if (validationError != null) {
                return ResponseEntity.status(400).body(validationError);
            }

            // Проверка на дубликат ID
            if (bookService.getBookById(book.getId()).isPresent()) {
                return ResponseEntity.status(400).body("Книга с таким ID уже существует");
            }

            bookService.addBook(book);
            return ResponseEntity.ok("Книга добавлена успешно");
        } catch (Exception e) {
            return ResponseEntity.status(400).body("Ошибка при добавлении: " + e.getMessage());
        }
    }

    /**
     * Изменить цену книги
     */
    @PutMapping("/books/{id}/price")
    public ResponseEntity<String> updatePrice(
            @PathVariable String id,
            @RequestParam double price,
            Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }

        try {
            // Валидация цены
            if (price < 0) {
                return ResponseEntity.status(400).body("Цена не может быть отрицательной");
            }

            if (price > 1000000) {
                return ResponseEntity.status(400).body("Цена слишком большая");
            }

            Optional<Book> bookOpt = bookService.getBookById(id);
            if (bookOpt.isEmpty()) {
                return ResponseEntity.status(404).body("Книга не найдена");
            }

            bookService.updatePrice(id, price);
            return ResponseEntity.ok("Цена обновлена");
        } catch (Exception e) {
            return ResponseEntity.status(400).body("Ошибка: " + e.getMessage());
        }
    }

    /**
     * Изменить количество книг (всего и в наличии)
     */
    @PutMapping("/books/{id}/quantity")
    public ResponseEntity<String> updateQuantity(
            @PathVariable String id,
            @RequestParam int total,
            @RequestParam int available,
            Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }

        try {
            Optional<Book> bookOpt = bookService.getBookById(id);
            if (bookOpt.isEmpty()) {
                return ResponseEntity.status(404).body("Книга не найдена");
            }

            if (available > total) {
                return ResponseEntity.status(400).body("Количество в наличии не может быть больше общего количества");
            }

            if (available < 0 || total < 0) {
                return ResponseEntity.status(400).body("Количество не может быть отрицательным");
            }

            if (total > 10000) {
                return ResponseEntity.status(400).body("Слишком большое количество экземпляров");
            }

            Book book = bookOpt.get();
            book.setTotal(total);
            book.setAvailable(available);
            bookService.updateBook(book);

            return ResponseEntity.ok("Количество обновлено");
        } catch (Exception e) {
            return ResponseEntity.status(400).body("Ошибка: " + e.getMessage());
        }
    }

    /**
     * Выдать книгу читателю
     */
    @PostMapping("/books/{bookId}/issue")
    public ResponseEntity<String> issueBook(
            @PathVariable String bookId,
            @RequestParam String readerUsername,
            Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }

        try {
            Optional<Book> book = bookService.getBookById(bookId);
            if (book.isEmpty()) {
                return ResponseEntity.status(404).body("Книга не найдена");
            }

            if (book.get().getAvailable() <= 0) {
                return ResponseEntity.status(400).body("Нет доступных экземпляров");
            }

            Optional<User> reader = userService.getUserByUsername(readerUsername);
            if (reader.isEmpty()) {
                return ResponseEntity.status(404).body("Читатель не найден");
            }

            if (reader.get().getRole() != UserRole.READER) {
                return ResponseEntity.status(400).body("Пользователь не является читателем");
            }

            bookService.issueBook(bookId);
            userService.addBorrowedBook(readerUsername, bookId);
            return ResponseEntity.ok("Книга выдана успешно");
        } catch (Exception e) {
            return ResponseEntity.status(400).body("Ошибка: " + e.getMessage());
        }
    }

    /**
     * Вернуть книгу от читателя
     */
    @PostMapping("/books/{bookId}/return")
    public ResponseEntity<String> returnBook(
            @PathVariable String bookId,
            @RequestParam String readerUsername,
            Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }

        try {
            Optional<Book> book = bookService.getBookById(bookId);
            if (book.isEmpty()) {
                return ResponseEntity.status(404).body("Книга не найдена");
            }

            Optional<User> reader = userService.getUserByUsername(readerUsername);
            if (reader.isEmpty()) {
                return ResponseEntity.status(404).body("Читатель не найден");
            }

            if (!reader.get().getBorrowedBookIds().contains(bookId)) {
                return ResponseEntity.status(400).body("У читателя нет этой книги");
            }

            bookService.returnBook(bookId);
            userService.returnBorrowedBook(readerUsername, bookId);
            return ResponseEntity.ok("Книга возвращена успешно");
        } catch (Exception e) {
            return ResponseEntity.status(400).body("Ошибка: " + e.getMessage());
        }
    }

    /**
     * Получить список всех читателей
     */
    @GetMapping("/readers")
    public ResponseEntity<List<User>> getAllReaders(Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }
        return ResponseEntity.ok(userService.getAllReaders());
    }

    /**
     * Получить детали читателя
     */
    @GetMapping("/readers/{username}")
    public ResponseEntity<User> getReaderDetails(
            @PathVariable String username,
            Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }

        Optional<User> reader = userService.getUserByUsername(username);
        return reader.map(ResponseEntity::ok)
                .orElseGet(() -> ResponseEntity.status(404).build());
    }

    /**
     * Валидация данных книги
     */
    private String validateBook(Book book) {
        // ID
        if (book.getId() == null || book.getId().trim().isEmpty()) {
            return "ID книги не может быть пустым";
        }
        if (book.getId().length() > 50) {
            return "ID книги слишком длинный";
        }
        if (!book.getId().matches("^[a-zA-Z0-9_-]+$")) {
            return "ID может содержать только буквы, цифры, дефис и подчеркивание";
        }

        // Название
        if (book.getTitle() == null || book.getTitle().trim().isEmpty()) {
            return "Название книги не может быть пустым";
        }
        if (book.getTitle().length() > 200) {
            return "Название книги слишком длинное";
        }

        // Авторы
        if (book.getAuthors() == null || book.getAuthors().trim().isEmpty()) {
            return "Авторы не могут быть пустыми";
        }
        if (book.getAuthors().length() > 200) {
            return "Список авторов слишком длинный";
        }

        // Год
        if (book.getYear() < 0) {
            return "Год не может быть отрицательным";
        }
        if (book.getYear() > 2100) {
            return "Год не может быть больше 2100";
        }

        // Категория
        if (book.getCategory() == null || book.getCategory().trim().isEmpty()) {
            return "Категория не может быть пустой";
        }
        if (book.getCategory().length() > 100) {
            return "Название категории слишком длинное";
        }

        // Цена
        if (book.getPrice() < 0) {
            return "Цена не может быть отрицательной";
        }
        if (book.getPrice() > 1000000) {
            return "Цена слишком большая";
        }

        // Количество
        if (book.getTotal() < 0) {
            return "Общее количество не может быть отрицательным";
        }
        if (book.getTotal() > 10000) {
            return "Слишком большое количество экземпляров";
        }
        if (book.getAvailable() < 0) {
            return "Количество в наличии не может быть отрицательным";
        }
        if (book.getAvailable() > book.getTotal()) {
            return "Количество в наличии не может быть больше общего количества";
        }

        return null; // Валидация пройдена
    }

    /**
     * Вспомогательный метод для проверки прав библиотекаря
     */
    private boolean isLibrarian(Authentication auth) {
        if (auth == null || !auth.isAuthenticated()) {
            return false;
        }

        Optional<User> user = userService.getUserByUsername(auth.getName());
        return user.isPresent() && user.get().getRole() == UserRole.LIBRARIAN;
    }

    /**
     * Получить все ожидающие заказы
     */
    @GetMapping("/orders/pending")
    public ResponseEntity<List<BookOrderService.BookOrder>> getPendingOrders(Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }
        return ResponseEntity.ok(bookOrderService.getPendingOrders());
    }

    /**
     * Получить все заказы
     */
    @GetMapping("/orders")
    public ResponseEntity<List<BookOrderService.BookOrder>> getAllOrders(Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }
        return ResponseEntity.ok(bookOrderService.getAllOrders());
    }

    /**
     * Одобрить заказ
     */
    @PostMapping("/orders/{orderId}/approve")
    public ResponseEntity<String> approveOrder(@PathVariable String orderId, Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }

        if (bookOrderService.approveOrder(orderId)) {
            return ResponseEntity.ok("Заказ одобрен и отправлен в обработку");
        } else {
            return ResponseEntity.status(400).body("Не удалось одобрить заказ");
        }
    }

    /**
     * Отклонить заказ
     */
    @PostMapping("/orders/{orderId}/reject")
    public ResponseEntity<String> rejectOrder(
            @PathVariable String orderId,
            @RequestParam(required = false) String reason,
            Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }

        if (bookOrderService.rejectOrder(orderId, reason)) {
            return ResponseEntity.ok("Заказ отклонён");
        } else {
            return ResponseEntity.status(400).body("Не удалось отклонить заказ");
        }
    }

}

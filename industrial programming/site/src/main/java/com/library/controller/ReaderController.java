package com.library.controller;

import com.library.model.Book;
import com.library.model.User;
import com.library.model.UserRole;
import com.library.service.BookService;
import com.library.service.BookOrderService;
import com.library.service.UserService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.scheduling.annotation.Async;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;

@RestController
@RequestMapping("/api/reader")
public class ReaderController {

    private static final Logger logger = LoggerFactory.getLogger(ReaderController.class);

    @Autowired
    private BookService bookService;

    @Autowired
    private UserService userService;

    @Autowired
    private BookOrderService bookOrderService;

    /**
     * Получить все книги (асинхронно в пуле потоков читателей)
     */
    @GetMapping("/books")
    @Async("readerSessionExecutor")
    public CompletableFuture<ResponseEntity<List<Book>>> getAllBooks(Authentication auth) {
        logger.info("[Поток: {}] Читатель {} запрашивает список книг",
                Thread.currentThread().getName(), auth.getName());

        if (!isReader(auth)) {
            return CompletableFuture.completedFuture(ResponseEntity.status(403).build());
        }
        return CompletableFuture.completedFuture(ResponseEntity.ok(bookService.getAllBooks()));
    }

    /**
     * Поиск книг по автору (асинхронно)
     */
    @GetMapping("/books/search/author")
    @Async("readerSessionExecutor")
    public CompletableFuture<ResponseEntity<List<Book>>> searchByAuthor(
            @RequestParam String author,
            Authentication auth) {
        logger.info("[Поток: {}] Читатель {} ищет книги по автору: {}",
                Thread.currentThread().getName(), auth.getName(), author);

        if (!isReader(auth)) {
            return CompletableFuture.completedFuture(ResponseEntity.status(403).build());
        }
        return CompletableFuture.completedFuture(
                ResponseEntity.ok(bookService.searchByAuthor(author))
        );
    }

    /**
     * Поиск книг по году (асинхронно)
     */
    @GetMapping("/books/search/year")
    @Async("readerSessionExecutor")
    public CompletableFuture<ResponseEntity<List<Book>>> searchByYear(
            @RequestParam int year,
            Authentication auth) {
        logger.info("[Поток: {}] Читатель {} ищет книги за год: {}",
                Thread.currentThread().getName(), auth.getName(), year);

        if (!isReader(auth)) {
            return CompletableFuture.completedFuture(ResponseEntity.status(403).build());
        }
        return CompletableFuture.completedFuture(
                ResponseEntity.ok(bookService.searchByYear(year))
        );
    }

    /**
     * Поиск книг по категории (асинхронно)
     */
    @GetMapping("/books/search/category")
    @Async("readerSessionExecutor")
    public CompletableFuture<ResponseEntity<List<Book>>> searchByCategory(
            @RequestParam String category,
            Authentication auth) {
        logger.info("[Поток: {}] Читатель {} ищет книги по категории: {}",
                Thread.currentThread().getName(), auth.getName(), category);

        if (!isReader(auth)) {
            return CompletableFuture.completedFuture(ResponseEntity.status(403).build());
        }
        return CompletableFuture.completedFuture(
                ResponseEntity.ok(bookService.searchByCategory(category))
        );
    }

    /**
     * Получить выданные книги читателя (асинхронно)
     */
    @GetMapping("/borrowed-books")
    @Async("readerSessionExecutor")
    public CompletableFuture<ResponseEntity<List<Book>>> getBorrowedBooks(Authentication auth) {
        logger.info("[Поток: {}] Читатель {} запрашивает свои книги",
                Thread.currentThread().getName(), auth.getName());

        if (!isReader(auth)) {
            return CompletableFuture.completedFuture(ResponseEntity.status(403).build());
        }

        Optional<User> user = userService.getUserByUsername(auth.getName());
        if (user.isEmpty()) {
            return CompletableFuture.completedFuture(ResponseEntity.status(404).build());
        }

        List<Book> borrowedBooks = user.get().getBorrowedBookIds().stream()
                .map(bookService::getBookById)
                .filter(Optional::isPresent)
                .map(Optional::get)
                .toList();

        return CompletableFuture.completedFuture(ResponseEntity.ok(borrowedBooks));
    }

    /**
     * Получить информацию об аккаунте (асинхронно)
     */
    @GetMapping("/account")
    @Async("readerSessionExecutor")
    public CompletableFuture<ResponseEntity<User>> getAccountInfo(Authentication auth) {
        logger.info("[Поток: {}] Читатель {} запрашивает информацию об аккаунте",
                Thread.currentThread().getName(), auth.getName());

        if (!isReader(auth)) {
            return CompletableFuture.completedFuture(ResponseEntity.status(403).build());
        }

        Optional<User> user = userService.getUserByUsername(auth.getName());
        return user.map(value -> CompletableFuture.completedFuture(ResponseEntity.ok(value)))
                .orElseGet(() -> CompletableFuture.completedFuture(ResponseEntity.status(404).build()));
    }

    /**
     * Заказать книгу
     */
    @PostMapping("/order-book")
    public ResponseEntity<Map<String, String>> orderBook(
            @RequestParam String bookId,
            Authentication auth) {
        logger.info("[Поток: {}] Читатель {} создает заказ на книгу {}",
                Thread.currentThread().getName(), auth.getName(), bookId);

        if (!isReader(auth)) {
            return ResponseEntity.status(403).build();
        }

        String orderId = bookOrderService.createOrder(auth.getName(), bookId);

        if (orderId == null) {
            return ResponseEntity.status(400).body(
                    Map.of("error", "Не удалось создать заказ")
            );
        }

        return ResponseEntity.ok(Map.of(
                "orderId", orderId,
                "message", "Заказ создан и отправлен библиотекарю"
        ));
    }

    /**
     * Проверить статус заказа
     */
    @GetMapping("/order-status/{orderId}")
    public ResponseEntity<BookOrderService.BookOrder> getOrderStatus(
            @PathVariable String orderId,
            Authentication auth) {
        if (!isReader(auth)) {
            return ResponseEntity.status(403).build();
        }

        BookOrderService.BookOrder order = bookOrderService.getOrderStatus(orderId);

        if (order == null) {
            return ResponseEntity.status(404).build();
        }

        if (!order.getUsername().equals(auth.getName())) {
            return ResponseEntity.status(403).build();
        }

        return ResponseEntity.ok(order);
    }

    private boolean isReader(Authentication auth) {
        if (auth == null || !auth.isAuthenticated()) {
            return false;
        }

        Optional<User> user = userService.getUserByUsername(auth.getName());
        return user.isPresent() && user.get().getRole() == UserRole.READER;
    }
}

package com.library.service;

import com.library.model.Book;
import com.library.model.User;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

@Service
public class BookOrderService {

    private static final Logger logger = LoggerFactory.getLogger(BookOrderService.class);

    @Autowired
    private BookService bookService;

    @Autowired
    private UserService userService;

    @Autowired
    private AsyncBookIssueService asyncBookIssueService;  // ← Новая зависимость

    // Хранилище заказов: orderId -> OrderStatus
    private final Map<String, BookOrder> orders = new ConcurrentHashMap<>();

    /**
     * Создать заказ на книгу
     */
    public String createOrder(String username, String bookId) {
        String orderId = UUID.randomUUID().toString();

        Optional<Book> book = bookService.getBookById(bookId);
        Optional<User> user = userService.getUserByUsername(username);

        if (book.isEmpty() || user.isEmpty()) {
            return null;
        }

        BookOrder order = new BookOrder(
                orderId,
                username,
                bookId,
                book.get().getTitle(),
                user.get().getFullName()
        );
        orders.put(orderId, order);

        logger.info("Заказ {} создан пользователем {} на книгу {}",
                orderId, username, book.get().getTitle());

        return orderId;
    }

    /**
     * Одобрить заказ (вызывается библиотекарем)
     */
    public boolean approveOrder(String orderId) {
        BookOrder order = orders.get(orderId);
        if (order == null || order.getStatus() != OrderStatus.PENDING) {
            return false;
        }

        logger.info("Библиотекарь одобрил заказ {}", orderId);

        // Запускаем асинхронную обработку заказа через ОТДЕЛЬНЫЙ сервис
        asyncBookIssueService.processOrderAsync(order);

        return true;
    }

    /**
     * Отклонить заказ (вызывается библиотекарем)
     */
    public boolean rejectOrder(String orderId, String reason) {
        BookOrder order = orders.get(orderId);
        if (order == null || order.getStatus() != OrderStatus.PENDING) {
            return false;
        }

        order.setStatus(OrderStatus.REJECTED);
        order.setMessage(reason != null ? reason : "Заказ отклонён библиотекарем");

        logger.info("Библиотекарь отклонил заказ {}: {}", orderId, order.getMessage());

        return true;
    }

    /**
     * Получить статус заказа
     */
    public BookOrder getOrderStatus(String orderId) {
        return orders.get(orderId);
    }

    /**
     * Получить все ожидающие заказы (для библиотекаря)
     */
    public List<BookOrder> getPendingOrders() {
        return orders.values().stream()
                .filter(o -> o.getStatus() == OrderStatus.PENDING)
                .collect(Collectors.toList());
    }

    /**
     * Получить все заказы (для библиотекаря)
     */
    public List<BookOrder> getAllOrders() {
        return new ArrayList<>(orders.values());
    }

    /**
     * Класс для хранения информации о заказе
     */
    public static class BookOrder {
        private final String orderId;
        private final String username;
        private final String userFullName;
        private final String bookId;
        private final String bookTitle;
        private final LocalDateTime createdAt;
        private OrderStatus status;
        private String message;

        public BookOrder(String orderId, String username, String bookId, String bookTitle, String userFullName) {
            this.orderId = orderId;
            this.username = username;
            this.bookId = bookId;
            this.bookTitle = bookTitle;
            this.userFullName = userFullName;
            this.createdAt = LocalDateTime.now();
            this.status = OrderStatus.PENDING;
            this.message = "Ожидание одобрения библиотекаря";
        }

        // Getters и setters
        public String getOrderId() { return orderId; }
        public String getUsername() { return username; }
        public String getUserFullName() { return userFullName; }
        public String getBookId() { return bookId; }
        public String getBookTitle() { return bookTitle; }
        public LocalDateTime getCreatedAt() { return createdAt; }
        public OrderStatus getStatus() { return status; }
        public void setStatus(OrderStatus status) { this.status = status; }
        public String getMessage() { return message; }
        public void setMessage(String message) { this.message = message; }
    }

    /**
     * Статусы заказа
     */
    public enum OrderStatus {
        PENDING,      // Ожидает одобрения библиотекаря
        PROCESSING,   // В обработке (библиотекарь готовит книгу)
        COMPLETED,    // Выполнен
        REJECTED      // Отклонен
    }
}

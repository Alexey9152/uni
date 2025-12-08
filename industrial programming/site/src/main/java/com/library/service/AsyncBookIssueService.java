package com.library.service;

import com.library.model.Book;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Service;

import java.util.Optional;
import java.util.concurrent.CompletableFuture;

@Service
public class AsyncBookIssueService {

    private static final Logger logger = LoggerFactory.getLogger(AsyncBookIssueService.class);

    @Autowired
    private BookService bookService;

    @Autowired
    private UserService userService;

    /**
     * Асинхронная обработка заказа библиотекарем
     */
    @Async("bookIssueExecutor")
    public CompletableFuture<Void> processOrderAsync(BookOrderService.BookOrder order) {
        try {
            logger.info("[Поток: {}] Библиотекарь начал обработку заказа {}",
                    Thread.currentThread().getName(), order.getOrderId());

            order.setStatus(BookOrderService.OrderStatus.PROCESSING);
            order.setMessage("Библиотекарь готовит книгу к выдаче");

            // Эмуляция работы библиотекаря (поиск книги на полке, проверка состояния и т.д.)
            Thread.sleep(3000 + (long)(Math.random() * 2000)); // 3-5 секунд

            // Проверяем доступность книги
            Optional<Book> book = bookService.getBookById(order.getBookId());
            if (book.isEmpty() || book.get().getAvailable() <= 0) {
                order.setStatus(BookOrderService.OrderStatus.REJECTED);
                order.setMessage("Книга недоступна");
                logger.warn("[Поток: {}] Заказ {} отклонен - книга недоступна",
                        Thread.currentThread().getName(), order.getOrderId());
                return CompletableFuture.completedFuture(null);
            }

            // Выдаем книгу
            bookService.issueBook(order.getBookId());
            userService.addBorrowedBook(order.getUsername(), order.getBookId());

            order.setStatus(BookOrderService.OrderStatus.COMPLETED);
            order.setMessage("Книга выдана успешно");

            logger.info("[Поток: {}] Заказ {} выполнен успешно",
                    Thread.currentThread().getName(), order.getOrderId());

        } catch (InterruptedException e) {
            order.setStatus(BookOrderService.OrderStatus.REJECTED);
            order.setMessage("Ошибка обработки заказа");
            logger.error("Ошибка при обработке заказа {}", order.getOrderId(), e);
            Thread.currentThread().interrupt();
        } catch (Exception e) {
            order.setStatus(BookOrderService.OrderStatus.REJECTED);
            order.setMessage("Ошибка: " + e.getMessage());
            logger.error("Ошибка при обработке заказа {}", order.getOrderId(), e);
        }

        return CompletableFuture.completedFuture(null);
    }
}

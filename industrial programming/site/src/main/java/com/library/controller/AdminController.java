package com.library.controller;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.http.ResponseEntity;
import org.springframework.scheduling.concurrent.ThreadPoolTaskExecutor;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executor;
import java.util.concurrent.ThreadPoolExecutor;

@RestController
@RequestMapping("/api/admin")
public class AdminController {

    @Autowired
    @Qualifier("readerSessionExecutor")
    private Executor readerSessionExecutor;

    @Autowired
    @Qualifier("bookIssueExecutor")
    private Executor bookIssueExecutor;

    /**
     * Получить статистику пула потоков для сессий читателей
     */
    @GetMapping("/threads/reader-sessions")
    public ResponseEntity<Map<String, Object>> getReaderSessionThreadPoolStats() {
        if (readerSessionExecutor instanceof ThreadPoolTaskExecutor) {
            ThreadPoolTaskExecutor executor = (ThreadPoolTaskExecutor) readerSessionExecutor;
            ThreadPoolExecutor threadPool = executor.getThreadPoolExecutor();

            Map<String, Object> stats = new HashMap<>();
            stats.put("poolName", "Reader Session Pool");
            stats.put("corePoolSize", executor.getCorePoolSize());
            stats.put("maxPoolSize", executor.getMaxPoolSize());
            stats.put("activeThreads", threadPool.getActiveCount());
            stats.put("poolSize", threadPool.getPoolSize());
            stats.put("queueSize", threadPool.getQueue().size());
            stats.put("completedTasks", threadPool.getCompletedTaskCount());
            stats.put("totalTasks", threadPool.getTaskCount());
            stats.put("queueCapacity", executor.getQueueCapacity());

            return ResponseEntity.ok(stats);
        }

        return ResponseEntity.status(500).body(Map.of("error", "Cannot get thread pool stats"));
    }

    /**
     * Получить статистику пула потоков для выдачи книг
     */
    @GetMapping("/threads/book-issue")
    public ResponseEntity<Map<String, Object>> getBookIssueThreadPoolStats() {
        if (bookIssueExecutor instanceof ThreadPoolTaskExecutor) {
            ThreadPoolTaskExecutor executor = (ThreadPoolTaskExecutor) bookIssueExecutor;
            ThreadPoolExecutor threadPool = executor.getThreadPoolExecutor();

            Map<String, Object> stats = new HashMap<>();
            stats.put("poolName", "Book Issue Pool");
            stats.put("corePoolSize", executor.getCorePoolSize());
            stats.put("maxPoolSize", executor.getMaxPoolSize());
            stats.put("activeThreads", threadPool.getActiveCount());
            stats.put("poolSize", threadPool.getPoolSize());
            stats.put("queueSize", threadPool.getQueue().size());
            stats.put("completedTasks", threadPool.getCompletedTaskCount());
            stats.put("totalTasks", threadPool.getTaskCount());
            stats.put("queueCapacity", executor.getQueueCapacity());

            return ResponseEntity.ok(stats);
        }

        return ResponseEntity.status(500).body(Map.of("error", "Cannot get thread pool stats"));
    }

    /**
     * Получить статистику всех пулов потоков
     */
    @GetMapping("/threads/all")
    public ResponseEntity<Map<String, Object>> getAllThreadPoolStats() {
        Map<String, Object> allStats = new HashMap<>();

        // Статистика пула сессий читателей
        if (readerSessionExecutor instanceof ThreadPoolTaskExecutor) {
            ThreadPoolTaskExecutor executor = (ThreadPoolTaskExecutor) readerSessionExecutor;
            ThreadPoolExecutor threadPool = executor.getThreadPoolExecutor();

            Map<String, Object> readerStats = new HashMap<>();
            readerStats.put("corePoolSize", executor.getCorePoolSize());
            readerStats.put("maxPoolSize", executor.getMaxPoolSize());
            readerStats.put("activeThreads", threadPool.getActiveCount());
            readerStats.put("poolSize", threadPool.getPoolSize());
            readerStats.put("queueSize", threadPool.getQueue().size());
            readerStats.put("completedTasks", threadPool.getCompletedTaskCount());
            readerStats.put("totalTasks", threadPool.getTaskCount());

            allStats.put("readerSessionPool", readerStats);
        }

        // Статистика пула выдачи книг
        if (bookIssueExecutor instanceof ThreadPoolTaskExecutor) {
            ThreadPoolTaskExecutor executor = (ThreadPoolTaskExecutor) bookIssueExecutor;
            ThreadPoolExecutor threadPool = executor.getThreadPoolExecutor();

            Map<String, Object> issueStats = new HashMap<>();
            issueStats.put("corePoolSize", executor.getCorePoolSize());
            issueStats.put("maxPoolSize", executor.getMaxPoolSize());
            issueStats.put("activeThreads", threadPool.getActiveCount());
            issueStats.put("poolSize", threadPool.getPoolSize());
            issueStats.put("queueSize", threadPool.getQueue().size());
            issueStats.put("completedTasks", threadPool.getCompletedTaskCount());
            issueStats.put("totalTasks", threadPool.getTaskCount());

            allStats.put("bookIssuePool", issueStats);
        }

        // Общая информация о потоках JVM
        allStats.put("jvmThreadCount", Thread.activeCount());
        allStats.put("jvmThreads", Thread.getAllStackTraces().keySet().stream()
                .map(Thread::getName)
                .filter(name -> name.startsWith("Reader-Session-") || name.startsWith("Librarian-Issue-"))
                .toList());

        return ResponseEntity.ok(allStats);
    }

    /**
     * Получить список всех активных потоков
     */
    @GetMapping("/threads/active")
    public ResponseEntity<Map<String, Object>> getActiveThreads() {
        Map<String, Object> result = new HashMap<>();

        Map<Thread, StackTraceElement[]> allThreads = Thread.getAllStackTraces();

        // Фильтруем только наши потоки
        var ourThreads = allThreads.keySet().stream()
                .filter(t -> t.getName().startsWith("Reader-Session-") ||
                        t.getName().startsWith("Librarian-Issue-"))
                .map(t -> {
                    Map<String, Object> threadInfo = new HashMap<>();
                    threadInfo.put("name", t.getName());
                    threadInfo.put("state", t.getState().toString());
                    threadInfo.put("priority", t.getPriority());
                    threadInfo.put("isAlive", t.isAlive());
                    threadInfo.put("isDaemon", t.isDaemon());
                    return threadInfo;
                })
                .toList();

        result.put("totalActiveThreads", Thread.activeCount());
        result.put("ourThreads", ourThreads);
        result.put("ourThreadsCount", ourThreads.size());

        return ResponseEntity.ok(result);
    }
}

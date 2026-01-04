package com.example.worker.messaging;

import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.rabbit.annotation.RabbitListener;
import org.springframework.stereotype.Component;

@Slf4j
@Component
public class TaskEventsListener {

    @RabbitListener(queues = "task.events.queue")
    public void handle(TaskEvent event) {
        log.info("Task event: {}", event);
    }
}

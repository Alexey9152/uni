package com.example.todo.messaging;

import org.springframework.amqp.rabbit.core.RabbitTemplate;
import org.springframework.stereotype.Service;

@Service
public class TaskEventPublisher {

    public static final String EXCHANGE = "task.events";

    private final RabbitTemplate rabbitTemplate;

    public TaskEventPublisher(RabbitTemplate rabbitTemplate) {
        this.rabbitTemplate = rabbitTemplate;
    }

    public void publish(String routingKey, TaskEvent event) {
        rabbitTemplate.convertAndSend(EXCHANGE, routingKey, event);
    }
}

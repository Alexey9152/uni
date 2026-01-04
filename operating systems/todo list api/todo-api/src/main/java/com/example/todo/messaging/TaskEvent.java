package com.example.todo.messaging;

import java.time.Instant;

public record TaskEvent(
        String type,        // created/updated/deleted
        Long taskId,
        String status,      // todo/in_progress/done
        Instant occurredAt
) {}

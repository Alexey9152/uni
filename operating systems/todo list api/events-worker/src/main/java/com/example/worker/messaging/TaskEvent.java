package com.example.worker.messaging;

import java.time.Instant;

public record TaskEvent(
        String type,
        Long taskId,
        String status,
        Instant occurredAt
) {}

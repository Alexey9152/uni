package com.example.todo.service;

import com.example.todo.dto.CreateTaskRequest;
import com.example.todo.dto.TaskDTO;
import com.example.todo.dto.UpdateTaskRequest;
import com.example.todo.exception.ResourceNotFoundException;
import com.example.todo.messaging.TaskEvent;
import com.example.todo.messaging.TaskEventPublisher;
import com.example.todo.model.Task;
import com.example.todo.model.Task.TaskStatus;
import com.example.todo.repository.TaskRepository;
import com.example.todo.util.TaskMapper;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.cache.annotation.CacheEvict;
import org.springframework.cache.annotation.Cacheable;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.time.LocalDateTime;
import java.util.List;

@Service
@Slf4j
@RequiredArgsConstructor
@Transactional(readOnly = true)
public class TaskService {

    private final TaskRepository taskRepository;
    private final TaskMapper taskMapper;
    private final TaskEventPublisher taskEventPublisher;

    public List<TaskDTO> getAllTasks(String status) {
        if (status != null && !status.isBlank()) {
            TaskStatus taskStatus = parseStatus(status);
            return taskRepository.findByStatus(taskStatus).stream()
                    .map(taskMapper::toDTO)
                    .toList();
        }
        return taskRepository.findAll().stream()
                .map(taskMapper::toDTO)
                .toList();
    }

    @Cacheable(value = "tasks", key = "#p0", unless = "#result == null")
    public TaskDTO getTaskById(Long id) {
        log.debug("Fetching task with ID: {}", id);

        Task task = taskRepository.findById(id)
                .orElseThrow(() -> new ResourceNotFoundException("Task not found with id: " + id));

        return taskMapper.toDTO(task);
    }

    @Transactional
    public TaskDTO createTask(CreateTaskRequest request) {
        log.info("Creating new task: {}", request.getTitle());

        Task task = Task.builder()
                .title(request.getTitle())
                .description(request.getDescription())
                .status(parseStatus(request.getStatus()))
                .createdAt(LocalDateTime.now())
                .updatedAt(LocalDateTime.now())
                .build();

        Task saved = taskRepository.save(task);

        taskEventPublisher.publish(
                "task.created",
                new TaskEvent("created", saved.getId(), saved.getStatus().name().toLowerCase(), Instant.now())
        ); // отправка через RabbitTemplate.convertAndSend(...) внутри publisher [web:911]

        log.info("Task created successfully with ID: {}", saved.getId());
        return taskMapper.toDTO(saved);
    }

    @CacheEvict(value = "tasks", key = "#p0")
    @Transactional
    public TaskDTO updateTask(Long id, UpdateTaskRequest request) {
        log.info("Updating task with ID: {}", id);

        Task task = taskRepository.findById(id)
                .orElseThrow(() -> new ResourceNotFoundException("Task not found with id: " + id));

        task.setTitle(request.getTitle());
        task.setDescription(request.getDescription());
        task.setStatus(parseStatus(request.getStatus())); // чтобы принимал todo/in_progress/done
        task.setUpdatedAt(LocalDateTime.now());

        Task saved = taskRepository.save(task);

        taskEventPublisher.publish(
                "task.updated",
                new TaskEvent("updated", saved.getId(), saved.getStatus().name().toLowerCase(), Instant.now())
        ); // отправка через RabbitTemplate.convertAndSend(...) [web:911]

        log.info("Task updated successfully");
        return taskMapper.toDTO(saved);
    }

    @CacheEvict(value = "tasks", key = "#p0")
    @Transactional
    public TaskDTO updateTaskStatus(Long id, String status) {
        Task task = taskRepository.findById(id)
                .orElseThrow(() -> new ResourceNotFoundException("Task not found with id: " + id));

        task.setStatus(parseStatus(status));
        task.setUpdatedAt(LocalDateTime.now());

        Task saved = taskRepository.save(task);

        taskEventPublisher.publish(
                "task.updated",
                new TaskEvent("updated", saved.getId(), saved.getStatus().name().toLowerCase(), Instant.now())
        ); // отправка через RabbitTemplate.convertAndSend(...) [web:911]

        return taskMapper.toDTO(saved);
    }

    @CacheEvict(value = "tasks", key = "#p0")
    @Transactional
    public void deleteTask(Long id) {
        log.info("Deleting task with ID: {}", id);

        if (!taskRepository.existsById(id)) {
            throw new ResourceNotFoundException("Task not found with id: " + id);
        }

        taskRepository.deleteById(id);

        taskEventPublisher.publish(
                "task.deleted",
                new TaskEvent("deleted", id, null, Instant.now())
        ); // отправка через RabbitTemplate.convertAndSend(...) [web:911]

        log.info("Task deleted successfully");
    }

    private TaskStatus parseStatus(String raw) {
        try {
            return TaskStatus.valueOf(raw.trim().toUpperCase());
        } catch (Exception e) {
            throw new IllegalArgumentException("Invalid status. Allowed: todo, in_progress, done");
        }
    }
}

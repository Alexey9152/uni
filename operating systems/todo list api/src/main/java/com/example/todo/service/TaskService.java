package com.example.todo.service;

import com.example.todo.dto.CreateTaskRequest;
import com.example.todo.dto.TaskDTO;
import com.example.todo.dto.UpdateTaskRequest;
import com.example.todo.exception.ResourceNotFoundException;
import com.example.todo.model.Task;
import com.example.todo.model.Task.TaskStatus;
import com.example.todo.repository.TaskRepository;
import com.example.todo.util.TaskMapper;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.cache.annotation.Cacheable;
import org.springframework.cache.annotation.CacheEvict;
import java.time.LocalDateTime;

@Service
@Slf4j
@RequiredArgsConstructor
@Transactional(readOnly = true)
public class TaskService {

    private final TaskRepository taskRepository;
    private final TaskMapper taskMapper;

    public Page<TaskDTO> getAllTasks(Pageable pageable, String status) {
        log.debug("Fetching all tasks with status: {}", status);

        if (status != null && !status.isEmpty()) {
            try {
                TaskStatus taskStatus = TaskStatus.valueOf(status.toUpperCase());
                return taskRepository.findByStatus(taskStatus, pageable)
                        .map(taskMapper::toDTO);
            } catch (IllegalArgumentException e) {
                throw new IllegalArgumentException("Invalid status: " + status);
            }
        }

        return taskRepository.findAll(pageable)
                .map(taskMapper::toDTO);
    }

    @Cacheable(value = "tasks", key = "#id", unless = "#result == null")
    public TaskDTO getTaskById(Long id) {
        log.debug("Fetching task with ID: {}", id);

        Task task = taskRepository.findById(id)
                .orElseThrow(() -> new ResourceNotFoundException(
                        "Task not found with id: " + id));

        return taskMapper.toDTO(task);
    }

    @Transactional
    public TaskDTO createTask(CreateTaskRequest request) {
        log.info("Creating new task: {}", request.getTitle());

        Task task = Task.builder()
                .title(request.getTitle())
                .description(request.getDescription())
                .status(TaskStatus.TODO)
                .createdAt(LocalDateTime.now())
                .updatedAt(LocalDateTime.now())
                .build();

        Task saved = taskRepository.save(task);
        log.info("Task created successfully with ID: {}", saved.getId());

        return taskMapper.toDTO(saved);
    }

    @CacheEvict(value = "tasks", key = "#id")
    @Transactional
    public TaskDTO updateTask(Long id, UpdateTaskRequest request) {
        log.info("Updating task with ID: {}", id);

        Task task = taskRepository.findById(id)
                .orElseThrow(() -> new ResourceNotFoundException(
                        "Task not found with id: " + id));

        if (request.getTitle() != null && !request.getTitle().isEmpty()) {
            task.setTitle(request.getTitle());
        }
        if (request.getDescription() != null) {
            task.setDescription(request.getDescription());
        }
        if (request.getStatus() != null && !request.getStatus().isEmpty()) {
            task.setStatus(TaskStatus.valueOf(request.getStatus().toUpperCase()));
        }

        task.setUpdatedAt(LocalDateTime.now());
        taskRepository.save(task);

        log.info("Task updated successfully");
        return taskMapper.toDTO(task);
    }

    @CacheEvict(value = "tasks", key = "#id")
    @Transactional
    public void deleteTask(Long id) {
        log.info("Deleting task with ID: {}", id);

        if (!taskRepository.existsById(id)) {
            throw new ResourceNotFoundException("Task not found with id: " + id);
        }

        taskRepository.deleteById(id);
        log.info("Task deleted successfully");
    }
}

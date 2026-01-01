# Этап 1: Сборка (Build)
FROM maven:3.9-eclipse-temurin-17 AS builder

WORKDIR /app

# Скачай зависимости
COPY pom.xml .
RUN mvn dependency:go-offline

# Скопируй исходный код и собери приложение
COPY src ./src
RUN mvn clean package -DskipTests

# Этап 2: Runtime (запуск)
FROM eclipse-temurin:17-jre-alpine

# Создай непривилегированного пользователя (для безопасности)
RUN addgroup -g 1001 -S appgroup && \
    adduser -u 1001 -S appuser -G appgroup

WORKDIR /app

# Скопируй собранный jar из этапа Builder
COPY --from=builder /app/target/*.jar app.jar

# Измени владельца
RUN chown -R appuser:appgroup /app
USER appuser

EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
    CMD wget --no-verbose --tries=1 --spider http://localhost:8080/actuator/health || exit 1

# Запусти приложение
ENTRYPOINT ["java", "-Dspring.profiles.active=docker", "-jar", "app.jar"]

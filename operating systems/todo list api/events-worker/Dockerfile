# ===== Stage 1: build =====
FROM maven:3.9-eclipse-temurin-17 AS builder
WORKDIR /workspace

# Копируем POM'ы (root + модули) чтобы кешировались зависимости
COPY pom.xml .
COPY todo-api/pom.xml todo-api/pom.xml
COPY gateway/pom.xml gateway/pom.xml
COPY events-worker/pom.xml events-worker/pom.xml

RUN mvn -q -DskipTests -pl :events-worker -am dependency:go-offline

# Копируем исходники только events-worker (минимум)
COPY events-worker/src events-worker/src

RUN mvn -q -DskipTests -pl :events-worker -am package

# ===== Stage 2: runtime =====
FROM eclipse-temurin:17-jre-alpine

RUN addgroup -g 1001 -S appgroup && \
    adduser -u 1001 -S appuser -G appgroup

WORKDIR /app

# worker обычно не принимает входящие http, но Actuator может быть включён -> порт можно не EXPOSE
COPY --from=builder /workspace/events-worker/target/*.jar app.jar

RUN chown -R appuser:appgroup /app
USER appuser

# Если включишь actuator на отдельном порту и захочешь healthcheck — добавишь wget/curl и HEALTHCHECK
ENTRYPOINT ["java", "-jar", "app.jar"]

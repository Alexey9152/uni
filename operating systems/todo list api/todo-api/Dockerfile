# ===== Stage 1: build =====
FROM maven:3.9-eclipse-temurin-17 AS builder
WORKDIR /workspace

# Копируем все pom.xml (root + модули) чтобы кешировались зависимости
COPY pom.xml .
COPY todo-api/pom.xml todo-api/pom.xml
COPY gateway/pom.xml gateway/pom.xml
COPY events-worker/pom.xml events-worker/pom.xml

RUN mvn -q -DskipTests -pl :todo-api -am dependency:go-offline

# Теперь копируем исходники (минимально нужные)
COPY todo-api/src todo-api/src

RUN mvn -q -DskipTests -pl :todo-api -am package

# ===== Stage 2: runtime =====
FROM eclipse-temurin:17-jre-alpine

RUN addgroup -g 1001 -S appgroup && \
    adduser -u 1001 -S appuser -G appgroup

# (Чтобы healthcheck всегда работал) ставим wget
RUN apk add --no-cache wget

WORKDIR /app
COPY --from=builder /workspace/todo-api/target/*.jar app.jar

RUN chown -R appuser:appgroup /app
USER appuser

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
  CMD wget -nv -t1 --spider http://localhost:8080/actuator/health || exit 1

ENTRYPOINT ["java", "-jar", "app.jar"]

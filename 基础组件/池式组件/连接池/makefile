# 编译器
CC = g++
# 编译选项
CFLAGS = -std=c++17 -Wall -g
# MySQL头文件路径（如果mysql.h不在系统路径，需指定，例如：-I/usr/local/mysql/include）
MYSQL_INC = 
# MySQL库路径（如果libmysqlclient.so不在系统路径，需指定，例如：-L/usr/local/mysql/lib）
MYSQL_LIB = 
# 链接库
LIBS = -lpthread -lmysqlclient

# 目标可执行文件
TARGET = mysql_pool
# 源文件
SRCS = main.cpp Connection.cpp ConnectionPool.cpp
# 目标文件
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 编译可执行文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(MYSQL_INC) $(MYSQL_LIB) -o $@ $^ $(LIBS)

# 编译目标文件
%.o: %.cpp
	$(CC) $(CFLAGS) $(MYSQL_INC) -c -o $@ $<

# 清理编译产物
clean:
	rm -rf $(TARGET) $(OBJS)

# 运行（可选）
run: $(TARGET)
	./$(TARGET)
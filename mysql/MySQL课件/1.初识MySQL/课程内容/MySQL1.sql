-- 创建数据库
CREATE DATABASE IF NOT EXISTS lesson DEFAULT CHARACTER SET GBK COLLATE GBK_CHINESE_CI;
-- 修改数据库
ALTER DATABASE lesson CHARACTER SET UTF8 COLLATE UTF8_GENERAL_CI;
-- 删除数据库
DROP DATABASE IF EXISTS lesson;
-- 查看数据库
SHOW DATABASES;
-- 使用数据库
USE lesson;

-- 列类型
decimal(5, 2); -- 成绩 "92.5" "100.00"

char(50); -- 不论插入的值占用多少位空间，在数据库中都会占50个长度 。比如"男"
varchar(50); -- 最大占用50个长度。比如 "男" 占用1个


-- 创建学生表，表中有字段学号、姓名、性别、年龄和成绩
CREATE TABLE IF NOT EXISTS student(
	`number` VARCHAR(30) NOT NULL PRIMARY KEY COMMENT '学号，主键',
	name VARCHAR(30) NOT NULL COMMENT '姓名',
	sex TINYINT(1) UNSIGNED DEFAULT 0 COMMENT '性别：0-男 1-女 2-其他',
	age TINYINT(3) UNSIGNED DEFAULT 0 COMMENT '年龄',
	score DOUBLE(5, 2) UNSIGNED COMMENT '成绩'
)ENGINE=InnoDB CHARSET=UTF8 COMMENT='学生表';

-- 将student表名称修改为stu
ALTER TABLE student RENAME AS stu;

-- 在stu表中添加字段联系电话(phone)，类型为字符串，长度为11，非空
ALTER TABLE stu ADD phone VARCHAR(11) NOT NULL COMMENT '联系电话';

-- 查看stu表结构
DESC stu;

--  将stu表中的sex字段的类型设置为VARCHAR，长度为2，默认值为'男'，
-- 注释为 "性别，男，女，其他"
ALTER TABLE stu MODIFY sex VARCHAR(2) DEFAULT '男' COMMENT '性别：男，女，其他';

-- 将stu表中phone字段修改为mobile，属性保持不变
ALTER TABLE stu CHANGE phone mobile VARCHAR(11) NOT NULL COMMENT '联系电话';

-- 将stu表中的mobile字段删除
ALTER TABLE  stu DROP mobile;

-- 删除数据表stu
DROP TABLE IF EXISTS stu;
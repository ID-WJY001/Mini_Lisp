# Mini Lisp 扩展功能

本文档描述了Mini Lisp解释器的扩展功能，包括新增的内置过程、用户体验改进、语法扩展。

## 1. 新增内置过程

### 1.1 字符串操作
- `string-append` - 字符串拼接
- `string-length` - 获取字符串长度
- `string-ref` - 获取指定位置的字符
- `number-string` - 数字转字符串
- `string-number` - 字符串转数字
```lisp
(string-append "foo" "bar") ; => "foobar"
(string-length "hello") ; => 5
(string-ref "abc" 1) ; => "b"
(number-string 123) ; => "123"
(string-number "456") ; => 456
```

### 1.2 字符串比较
```lisp
(string=? "hello" "hello")  ; => #t
(string<? "abc" "def")      ; => #t
(string>? "def" "abc")      ; => #t
```

### 1.3 字符串转换
```lisp
(string-upcase "hello")     ; => "HELLO"
(string-downcase "HELLO")   ; => "hello"
```

### 1.4 字符串子串
```lisp
(substring "Hello World" 0 5)  ; => "Hello"
```

### 1.5 输入输出扩展
```lisp
(readline)  ; 读取一行输入，例如输入 123，返回 "123"
(read)      ; 读取一个表达式，例如输入 (+ 1 2)，返回 (+ 1 2)
```

## 2. 用户体验改进

### 2.1 多行输入支持
```lisp
>>> (define (f x)
...         (* x x))
()
>>> (f 2)
4
```

## 3. 语法扩展

### 3.1 多行注释
```lisp
#|
这是一个
多行注释
|#
(+ 1 2) ; => 3
```

### 3.2 宏
```lisp
(define-macro unless (condition body)
  `(if (not ,condition) ,body))

(unless (> 3 4) (display "3 is not greater than 4"))
```-macro 
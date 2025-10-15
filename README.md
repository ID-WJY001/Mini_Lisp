## Mini-Lisp 解释器

这是一个为北京大学《[软件设计实践](https://pku-software.github.io)》课程练习而实现的简易 Lisp 解释器，功能集中在词法/语法解析、求值环境、少量 I/O 与常用过程，力求代码清晰易读，方便课堂使用与自测。**使用CMake运行，具体问题请查询课程网站。**

## 功能概览

- REPL 与脚本执行：
  - 直接启动进入 REPL，支持括号计数的多行输入提示（`>>>` / `...`）。
  - 传入一个文件路径参数可按序执行文件内的表达式（默认不打印结果，需用 `display`/`print`）。
- 数据类型：数字（双精度）、布尔（`#t`/`#f`）、字符串、符号、对与表（pair/list）、空表 `()`。
- 注释：行注释 `; ...`，块注释 `#| ... |#`。
- 真值规则：仅 `#f` 为假，`()` 也被视为真（和传统 Scheme 一致）。
- 语法特性：支持有点对的表（dotted pair），支持 `quote`/`quasiquote`/`unquote`。

## 特殊形式（Special Forms）

- define：变量与函数定义
- lambda：创建过程（闭包）
- if：条件分支（2 或 3 个分支）
- begin：顺序执行，返回最后一个表达式的值
- let：创建局部绑定后计算主体
- cond：条件分支，支持 `else`
- and / or：短路逻辑，返回最后一个求值结果或第一个真值
- quote / quasiquote / unquote：引用与模板展开（`unquote` 仅在 `quasiquote` 内有效）
- define-macro：简单宏定义（将实参以语法树形式绑定，再展开求值）

实现位置：`src/forms.cpp` 与 `src/eval_env.cpp`（特殊形式分派）。

## 内建过程（Builtins）

按类别列一个实用且不夸张的子集，完整映射可见 `src/builtins.cpp`：

- I/O 与控制：
  - `display` `displayln` `newline` `print`
  - `readline` `read` `read-multiline`
  - `eval` `apply` `exit` `error`
- 断言/类型判断：
  - `atom?` `boolean?` `integer?` `list?` `number?` `null?` `pair?` `procedure?` `string?` `symbol?`
- 列表处理：
  - `cons` `car` `cdr` `append` `length` `list`
  - 高阶：`map` `filter` `reduce`
- 数值与比较：
  - `+` `-` `*` `/` `abs` `expt` `quotient` `modulo` `remainder`
  - 比较：`>` `<` `=` `>=` `<=` `eq?` `equal?` `not` `even?` `odd?` `zero?`
- 字符串：
  - 构造/信息：`string-append` `string-length` `string-ref`
  - 比较：`string=?` `string<?` `string>?`
  - 转换：`string-upcase` `string-downcase` `substring`
  - 数字互转（注意名称）：`number-string`，`string-number`

说明与约束：

- `/`、`quotient`、`modulo`、`remainder` 对 0 做检查；`integer?` 通过小数部分为 0 判断。
- `map`/`filter` 的第二参需为 proper list；`reduce` 需要非空表。
- `eq?` 为同一对象判等，数字做近似比较；`equal?` 递归结构相等。
- 字符串转义目前仅支持 `\n` 与 `\"` 等简单形式。

## 快速示例

```lisp
; 算术与列表
(+ 1 2 3)                    ; => 6
(list 1 2 3)                 ; => (1 2 3)
(append (list 1) (list 2 3)) ; => (1 2 3)

; 定义变量与函数
(define x 10)
(define (add a b) (+ a b))
(add x 5)                    ; => 15

; 条件与 let
(if (> x 5) "big" "small") ; => "big"
(let ((a 1) (b 2)) (+ a b))  ; => 3

; 高阶过程
(map (lambda (n) (* n n)) (list 1 2 3)) ; => (1 4 9)
(filter odd? (list 1 2 3 4))            ; => (1 3)
(reduce + (list 1 2 3 4))               ; => 10

; quasiquote / unquote（模板展开）
(quasiquote (1 (unquote (+ 1 2)) 3))    ; => (1 3 3)

; 宏（非常简化的演示）
(define-macro unless (cond body)
  (quasiquote (if (not (unquote cond)) (begin (unquote body)) ())) )
(unless #f (displayln "works"))
```

## 设计要点（简述）

- 词法分析：`Tokenizer`，支持行/块注释与字符串字面量。
- 语法分析：`Parser`，生成由 `Value` 派生类组成的语法树，支持点对与特殊记号（quote 等）。
- 运行时：`EvalEnv`（带父环境的链式作用域），特殊形式直通分派；过程包括内建过程与闭包（`LambdaValue`）。
- 值体系：数字/布尔/字符串/符号/对/空表/过程/宏等，列表通过 `PairValue` 表示。

## 已知限制

- 数字使用 `double` 表示，存在精度与比较边界；未实现大整数、精确有理数语法等。
- 未保证尾递归优化；大深度递归可能导致栈溢出。
- I/O 与错误处理较简化；`exit`/`error` 会直接终止或抛异常。
- 标准库极少，仅适配课程练习需要。

—— 仅作课程作业与练习使用，代码还有改进空间，欢迎理性建议。**严禁抄袭代码，否则后果自负。**

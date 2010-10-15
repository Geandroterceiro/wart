(load "bdd.lisp")

(defun foo() 34)
(test "simple defun"
  :valueof (foo)
  :should be 34)

(progn
  (defun foo0() 3)
  (defun foo1() (+ 1 (foo0))))
(test "nested defuns"
  :valueof (foo1)
  :should be 4)

(progn
  (defmacro foo2(n) `(+ 1 ,n))
  (defmacro foo3(n) `(+ 1 (foo2 ,n))))
(test "nested macros"
  :valueof (foo3 2)
  :should be 4)

(setq foofoo (lambda() 3))
(test "no need for funcall on function atoms"
  :valueof (foofoo)
  :should be 3)

(test "no need for funcall on function forms"
  :valueof ((lambda() 3))
  :should be 3)

(test "no need for funcall on function forms with args"
  :valueof ((lambda(a) (+ a 3)) 2)
  :should be 5)

(eval (wc '(def foo4() 34)))
(test "simple def"
  :valueof (foo4)
  :should be 34)

(eval (wc '(mac foo5(n) `(+ ,n 1))))
(test "simple mac"
  :valueof (foo5 32)
  :should be 33)

(eval (wc '(def foo6() (cons 3 4))))
(test "def 2"
  :valueof (foo6)
  :should be (cons 3 4))

(eval (wc '(def foo7(a . b) b)))
(test "dotted rest"
  :valueof (foo7 3 4)
  :should be '(4))

(test "wc-destructuring-bind handles plain vars"
  :valueof (wc-destructuring-bind a 1 a)
  :should be 1)

(test "wc-destructuring-bind handles nested lists"
  :valueof (wc-destructuring-bind (a) '(1) a)
  :should be 1)

(test "wc-destructuring-bind handles empty lists"
  :valueof (wc-destructuring-bind (a) '() a)
  :should be nil)

(test "wc-destructuring-bind handles lists"
  :valueof (wc-destructuring-bind ((a b) (c d)) '((1 2) (3 4)) c)
  :should be 3)

(test "wc-destructuring-bind handles &rest"
  :valueof (wc-destructuring-bind (&rest b) '(1) b)
  :should be '(1))

(test "wc-destructuring-bind handles destructured args + rest"
  :valueof (wc-destructuring-bind ((a b) (c d) &rest e) '((1 2) (3 4) 5 6 7) (cons c e))
  :should be '(3 5 6 7))

(eval (wc '(def foo8((a b)) b)))
(test "destructured args"
  :valueof (foo8 '(3 4))
  :should be 4)

(eval (wc '(def foo9((a b) . c) b)))
(test "destructured args + dotted rest"
  :valueof (foo9 '(3 4) 5)
  :should be 4)

(test "destructured args + dotted rest for fn"
  :valueof ((fn ((a b) . c) b) '(3 4) 5)
  :should be 4)

(test "alternative syntax for fn"
  :valueof ([+ _ 1] 3)
  :should be 4)

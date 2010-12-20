;; Extensible coercion

(setf *wart-coercions* (table))

(defun wart-coerce(val dest)
  (or (aand (gethash dest *wart-coercions*)
            (gethash (type* val) it)
            (funcall it (val* val)))
      (funcall #'coerce val dest)))
(defover coerce wart-coerce)

(defmacro defcoerce(src dest converter)
  `(progn
     (unless (gethash ,dest *wart-coercions*)
       (setf (gethash ,dest *wart-coercions*) (table)))
     (setf (gethash ,src (gethash ,dest *wart-coercions*))
           ,converter)))

(defun isa(elem typ)
  (is typ (type-of elem)))



;; Internals

(defun type*(val)
  (if (and (isa val 'symbol) (not (fboundp val)))
    (type* (eval val))
    (type-of val)))

(defun val*(val)
  (if (isa val 'symbol)
    (val* (eval val))
    val))

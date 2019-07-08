(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (=  skoX 0) (<  (+ (+ (+ 3 (* skoX skoX)) (* skoY (+ (* skoX (- 4)) (* skoY (+ 1 (* skoX (* skoX 3))))))) (* skoZ (+ (+ (* skoX 2) (* skoY (+ (+ 2 (* skoX (* skoX (- 2)))) (* skoY (* skoX (- 2)))))) (* skoZ (+ 1 (* skoY (+ (* skoX (- 2)) (* skoY (* skoX skoX))))))))) 0) (<  (+ (+ (* skoX (* skoX skoX)) (* skoY (* skoY (+ (* skoX (* skoX (* skoX 3))) (* skoY (+ 1 (* skoX (* skoX 3)))))))) (* skoZ (+ (+ (+ (- 3) (* skoX (* skoX 2))) (* skoY (+ (* skoX (+ 10 (* skoX (* skoX (- 2))))) (* skoY (+ (+ 2 (* skoX (* skoX (- 7)))) (* skoY (* skoX (- 2)))))))) (* skoZ (+ skoX (* skoY (+ (+ 1 (* skoX (* skoX (- 2)))) (* skoY (+ (* skoX (+ (- 2) (* skoX skoX))) (* skoY (* skoX skoX))))))))))) 0) (=  skoY 0) (=  (+ (+ (* skoX (- 1)) (* skoY (- 1))) (* skoZ (+ (- 1) (* skoY skoX)))) 0) (>  skoZ 0) (<  (+ (- 1) skoY) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)
(set-info :smt-lib-version 2.0)
(set-logic QF_NRA)
(set-info :source |Produced by tarski version 1.30|)
(set-info :license "https://creativecommons.org/licenses/by/4.0/")
(set-info :category "crafted")
(set-info :status unknown)
(declare-fun v6 () Real)
(declare-fun w1 () Real)
(declare-fun v5 () Real)
(assert (and (not (=  v6 0)) (>  w1 0) (= (* (- 1) (+ (+ (* v6 (* v6 (+ 4 (* v6 (* v6 4))))) (* w1 (* w1 (* v6 (* v6 (- 729)))))) (* v5 (+ (* v6 (* v6 (- 8))) (* v5 (+ (+ 4 (* v6 (* v6 8))) (* v5 (+ (- 8) (* v5 4))))))))) 0) (< (* (- 1) (+ (- 1) (* w1 2)) (+ (+ (* v6 (* v6 (+ 8 (* v6 (* v6 8))))) (* w1 (* v6 (* v6 (- 1458))))) (* v5 (+ (* v6 (* v6 713)) (* v5 (+ (+ 8 (* v6 (* v6 16))) (* v5 (+ (- 16) (* v5 8))))))))) 0) (< (* (+ (+ (* v6 (* v6 (+ 8 (* v6 (* v6 8))))) (* w1 (* v6 (* v6 (- 1458))))) (* v5 (+ (* v6 (* v6 713)) (* v5 (+ (+ 8 (* v6 (* v6 16))) (* v5 (+ (- 16) (* v5 8)))))))) (+ (+ (* v6 (* v6 (+ (- 721) (* v6 (* v6 8))))) (* w1 (+ (* v6 (* v6 1458)) (* w1 (* v6 (* v6 (- 2916))))))) (* v5 (+ (* v6 (* v6 713)) (* v5 (+ (+ 8 (* v6 (* v6 16))) (* v5 (+ (- 16) (* v5 8))))))))) 0) (=  (+ (+ (* v6 (* v6 (* v6 (* v6 (+ 11728 (* v6 (* v6 (+ (- 531313) (* v6 (* v6 (- 11600))))))))))) (* w1 (* v6 (* v6 (* v6 (* v6 (+ (- 23328) (* v6 (* v6 2102436))))))))) (* v5 (+ (+ (* v6 (* v6 (* v6 (* v6 (+ (- 11920) (* v6 (* v6 34736))))))) (* w1 (* v6 (* v6 (* v6 (* v6 (- 2079108))))))) (* v5 (+ (+ (* v6 (* v6 (+ 11792 (* v6 (* v6 (+ (- 22688) (* v6 (* v6 (- 34736))))))))) (* w1 (* v6 (* v6 (+ (- 23328) (* v6 (* v6 2079108))))))) (* v5 (+ (+ (* v6 (* v6 (+ (- 12176) (* v6 (* v6 69216))))) (* w1 (* v6 (* v6 46656)))) (* v5 (+ (+ (+ 64 (* v6 (* v6 (+ (- 22432) (* v6 (* v6 (- 34608))))))) (* w1 (* v6 (* v6 (- 23328))))) (* v5 (+ (+ (- 256) (* v6 (* v6 34224))) (* v5 (+ (+ 384 (* v6 (* v6 (- 11408)))) (* v5 (+ (- 256) (* v5 64)))))))))))))))) 0)))
(check-sat)

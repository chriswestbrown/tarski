(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoSS () Real)
(declare-fun skoX () Real)
(declare-fun skoSM () Real)
(assert (and (<= (* (- 1) (+ (+ (- 1914) (* skoSS (- 957))) (* skoX 500))) 0) (>  (+ (+ (- 1914) (* skoSS (- 957))) (* skoX 500)) 0) (> (* (- 1) (+ (- 1) skoX)) 0) (>  skoX 0) (<= (* (- 1) skoSM) 0) (<= (* (- 1) skoSS) 0) (=  (+ (+ (- 1) skoX) (* skoSM skoSM)) 0) (=  (+ (+ (- 1) (* skoSS skoSS)) (* skoX skoX)) 0)))
(check-sat)

(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoSMX () Real)
(declare-fun skoSX () Real)
(declare-fun skoX () Real)
(assert (and (>  (+ (+ (* skoSMX (- 12)) (* skoSX 12)) (* skoX (+ (+ (+ (- 8) (* skoSMX (- 2))) (* skoSX (- 2))) (* skoX (+ (+ (* skoSMX 8) (* skoSX (- 8))) (* skoX (+ (+ 4 skoSMX) skoSX))))))) 0) (>  (+ (- 2) (* skoX skoX)) 0) (>  (+ (+ (* skoSMX 6) (* skoSX (- 6))) (* skoX (+ (+ (+ 4 skoSMX) skoSX) (* skoX (+ (* skoSMX (- 2)) (* skoSX 2)))))) 0) (>  skoX 0) (=  skoSMX 0) (>  skoSX 0) (=  (+ (+ 1 (* skoSX (* skoSX (- 1)))) skoX) 0) (=  (+ (+ (- 1) (* skoSMX skoSMX)) skoX) 0)))
(check-sat)

(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoCOSS () Real)
(declare-fun skoS () Real)
(declare-fun skoSINS () Real)
(declare-fun pi () Real)
(assert (and (=  (+ (+ (+ (- 2) (* skoCOSS (+ 2 (* skoCOSS 2)))) (* skoS (+ (* skoCOSS (+ 10 (* skoCOSS 2))) (* skoS (+ (+ 6 (* skoCOSS 6)) (* skoS 2)))))) (* skoSINS (+ (+ (+ (- 3) (* skoCOSS (- 2))) (* skoS (+ (- 4) (* skoS (+ 2 skoS))))) (* skoSINS (+ 1 skoS))))) 0) (>  (+ (* skoS (- 2)) pi) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (=  skoS 0) (=  skoCOSS 0) (=  (+ (* skoS (- 1)) skoSINS) 0)))
(check-sat)

(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun h1 () Real)
(declare-fun h2 () Real)
(declare-fun h3 () Real)
(declare-fun h5 () Real)
(declare-fun h6 () Real)
(declare-fun j2 () Real)
(assert (and (>  h1 0) (>  h2 0) (>  h3 0) (>  h5 0) (>  h6 0) (>  j2 0) (= (* j2 (+ h5 h6) (+ (+ h1 h2) (* j2 h6)) (+ (* h3 4) (* j2 (+ (* h3 4) h6))) (+ (+ (+ h1 h2) (* h3 4)) (* j2 (* h3 4))) (+ (* h3 4) (* j2 (+ (* h3 4) h5))) (+ (+ h1 h2) (* j2 h5))) 0)))
(check-sat)

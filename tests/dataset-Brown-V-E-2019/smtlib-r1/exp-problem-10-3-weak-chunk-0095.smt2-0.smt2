(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoC () Real)
(declare-fun skoCM1 () Real)
(declare-fun skoX () Real)
(assert (and (> (* (- 1) (+ 1 (* skoC (+ (- 6) (* skoC 12))))) 0) (> (* (- 1) (+ (+ (- 1) (* skoC (+ 6 (* skoC (- 12))))) (* skoCM1 (+ (+ (- 45) (* skoC (+ 270 (* skoC (- 540))))) (* skoCM1 (+ (+ (- 1215) (* skoC (+ 7290 (* skoC (- 14580))))) (* skoCM1 (+ (+ (- 24435) (* skoC (+ 146610 (* skoC (- 293220))))) (* skoCM1 (+ (+ (- 393660) (* skoC (+ 2361960 (* skoC (- 4723920))))) (* skoCM1 (+ (+ (- 5263380) (* skoC (+ 31580280 (* skoC (- 63160560))))) (* skoCM1 (+ (+ (- 59632200) (* skoC (+ 357793200 (* skoC (- 715586400))))) (* skoCM1 (+ (+ (- 578680200) (* skoC (+ 3472081200 (* skoC (- 6944162400))))) (* skoCM1 (+ (+ (- 4818398400) (* skoC (+ 28910390400 (* skoC (- 57820780800))))) (* skoCM1 (+ (+ (- 34295659200) (* skoC (+ 205773955200 (* skoC (- 411547910400))))) (* skoCM1 (+ (+ (- 206624260800) (* skoC (+ 1239745564800 (* skoC (- 2479491129600))))) (* skoCM1 (+ (+ (- 1033121304000) (* skoC (+ 6198727824000 (* skoC (- 12397455648000))))) (* skoCM1 (+ (+ (- 4132485216000) (* skoC (+ 24794911296000 (* skoC (- 49589822592000))))) (* skoCM1 (+ (+ (- 12397455648000) (* skoC (+ 74384733888000 (* skoC (- 148769467776000))))) (* skoCM1 (+ (+ (- 24794911296000) (* skoC (+ 148769467776000 (* skoC (- 297538935552000))))) (* skoCM1 (* skoC 297538935552000)))))))))))))))))))))))))))))))) 0) (>  (+ (- 1) skoX) 0) (>  skoCM1 0) (>  skoC 0)))
(check-sat)

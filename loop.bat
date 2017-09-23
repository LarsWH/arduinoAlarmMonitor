javac ClientMain.java
:loop
java ClientMain ENC28JBE0003 input
timeout /t 2 /nobreak
goto loop
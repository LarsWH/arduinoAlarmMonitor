wsl

ip=192.168.0.33
name=ENC28JBE0003
port=12900

ping -c 4 $ip
ping -c 4 $name

# TCP connnection
nc $ip $port
input
output
output 4 1

# Green
output 14 1
output 14 0

# Yellow
output 15 1
output 15 0

# Red
output 16 1
output 16 0

# Ctrl-c to exit

## UDP connnection
#nc -u $ip $port
#input
## Ctrl-c to exit

telnet $name $port
Exit: Ctrl-å + quit


cd '/mnt/d/Users/lars/Documents/Visual Studio 2017/Projects/Arduino/arduinoAlarmMonitor'

javac ClientMain.java
java ClientMain ENC28JBE0003 input


sudo apt-get update
sudo apt install openjdk-11-jdk
curl -X POST -H "Content-Type: text/plain" --data "input" http://$ip

exit
cmd
"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avr-g++" --version
# avr-g++ (GCC) 7.3.0

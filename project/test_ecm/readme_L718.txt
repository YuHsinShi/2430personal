

註1：FIBOCOM L718 4G模塊的撥號流程可透過UART介面來設定，也可透過USB OPTION驅動來設定。

註2：若使用USB OPTION驅動來撥號，撥號的程序可參考test_dial_io_mode.c裡面的Dial_io_mode_L718()。
     設定可參考Kconfg.L718

註3：使用前需先確認4G模塊有接上天線(ANT_MAIN)，且有插上SIM卡。

註4：使用FIBOCOM L718 4G模塊需先提早給它上電。




若使用PC透過UART來播號，指令如下：


◎若L718 4G模塊為第一次使用，需先跑過下面流程

# 確認數據端口能通信
ATE0
OK

# 確認SIM卡能被正常識別
AT+CPIN?
READY

# 查看當前時間、當前環境、當前天線的條件下的SIM卡所屬營運商網路制式的信號質量參考
# 若第一個參數 < 12，需接天線，如果只用一根天線，要接MAIN_ANT，最好選信號好的地方
AT+CSQ
+CSQ: 22,99
OK

# 確認數據服務是否可用
AT+CGREG?
+CGREG: 1,1
OK

# 檢查網路模式，7表示是4G
AT+COPS?
+COPS: 0,0,"T Star",7
OK

# 設置APN
AT+CGDCONT=1,"IP","internet"
OK

# ECM撥號
AT+GTRNDIS=1,1
OK

# 確認撥號成功，且取得IP
AT+GTRNDIS?
+GTRNDIS: 1,1,10.149.198.114,172.24.9.21,10.9.121.102
OK



◎ 若非第一次使用，每次測試前請先確認模塊已撥號成功

# 確認播號是否成功使用
AT+GTRNDIS?

L718回應如下即已經撥號成功
+GTRNDIS: 1,1,10.105.9.201,172.24.9.21,10.9.121.102
OK

L718回應如下即尚未撥號成功
+GTRNDIS: 0
OK

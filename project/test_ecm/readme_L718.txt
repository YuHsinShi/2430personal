

��1�GFIBOCOM L718 4G�Ҷ��������y�{�i�z�LUART�����ӳ]�w�A�]�i�z�LUSB OPTION�X�ʨӳ]�w�C

��2�G�Y�ϥ�USB OPTION�X�ʨӼ����A�������{�ǥi�Ѧ�test_dial_io_mode.c�̭���Dial_io_mode_L718()�C
     �]�w�i�Ѧ�Kconfg.L718

��3�G�ϥΫe�ݥ��T�{4G�Ҷ������W�ѽu(ANT_MAIN)�A�B�����WSIM�d�C

��4�G�ϥ�FIBOCOM L718 4G�Ҷ��ݥ����������W�q�C




�Y�ϥ�PC�z�LUART�Ӽ����A���O�p�U�G


���YL718 4G�Ҷ����Ĥ@���ϥΡA�ݥ��]�L�U���y�{

# �T�{�ƾںݤf��q�H
ATE0
OK

# �T�{SIM�d��Q���`�ѧO
AT+CPIN?
READY

# �d�ݷ�e�ɶ��B��e���ҡB��e�ѽu������U��SIM�d������B�Ӻ�������H����q�Ѧ�
# �Y�Ĥ@�ӰѼ� < 12�A�ݱ��ѽu�A�p�G�u�Τ@�ڤѽu�A�n��MAIN_ANT�A�̦n��H���n���a��
AT+CSQ
+CSQ: 22,99
OK

# �T�{�ƾڪA�ȬO�_�i��
AT+CGREG?
+CGREG: 1,1
OK

# �ˬd�����Ҧ��A7��ܬO4G
AT+COPS?
+COPS: 0,0,"T Star",7
OK

# �]�mAPN
AT+CGDCONT=1,"IP","internet"
OK

# ECM����
AT+GTRNDIS=1,1
OK

# �T�{�������\�A�B���oIP
AT+GTRNDIS?
+GTRNDIS: 1,1,10.149.198.114,172.24.9.21,10.9.121.102
OK



�� �Y�D�Ĥ@���ϥΡA�C�����իe�Х��T�{�Ҷ��w�������\

# �T�{�����O�_���\�ϥ�
AT+GTRNDIS?

L718�^���p�U�Y�w�g�������\
+GTRNDIS: 1,1,10.105.9.201,172.24.9.21,10.9.121.102
OK

L718�^���p�U�Y�|���������\
+GTRNDIS: 0
OK

% lp_filter_sim.m - 修正版
clc;
clear;
close all;

% % 7Mhz
% C1 = rfckt.shuntrlc('C', 680e-12);
% L1 = rfckt.seriesrlc('L', 1.5e-6);
% C2 = rfckt.shuntrlc('C', 1e-9);
% L2 = rfckt.seriesrlc('L', 1.5e-6);
% C3 = rfckt.shuntrlc('C', 1e-9);
% L3 = rfckt.seriesrlc('L', 1.5e-6);
% C4 = rfckt.shuntrlc('C', 680e-12);

% % 10Mhz
% C1 = rfckt.shuntrlc('C', 470e-12);
% L1 = rfckt.seriesrlc('L', 1e-6);
% C2 = rfckt.shuntrlc('C', 680e-12);
% L2 = rfckt.seriesrlc('L', 1e-6);
% C3 = rfckt.shuntrlc('C', 680e-12);
% L3 = rfckt.seriesrlc('L', 1e-6);
% C4 = rfckt.shuntrlc('C', 470e-12);


% 10Mhz test
C1 = rfckt.shuntrlc('C', 330e-12);
L1 = rfckt.seriesrlc('L', 680e-9);
C2 = rfckt.shuntrlc('C', 330e-12);

% 构建电路
ckt = rfckt.cascade('Ckts', {C1, L1, C2});

% 频率范围
f = linspace(1e6, 30e6, 1000);
analyze(ckt, f);

% 提取 S 参数数据
sparams_obj = ckt.AnalyzedResult.S_Parameters;

% 转换为 sparameters 对象
s = sparameters(sparams_obj, f, 50);  % 50Ω系统

% 绘制 S21 传输参数
figure;
subplot(1,2,1);
rfplot(s, 2, 1);  % S21
grid on;
title('S_{21} - 传输系数');
xlabel('频率 (Hz)');
ylabel('S_{21} (dB)');

% 绘制 S11 输入反射参数
subplot(1,2,2);
rfplot(s, 1, 1);  % S11
grid on;
title('S_{11} - 输入反射');
xlabel('频率 (MHz)');
ylabel('S_{11} (dB)');

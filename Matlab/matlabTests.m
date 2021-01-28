a = str2double(extractAfter(TCPtestdata(:,1), 1));
b = TCPtestdata(:,2);
% inputFormat = 'yyyy.MM.dd HH:mm:ss.SSS';
inputFormat = 'mm:ss.SSS';
c = datetime(b,'InputFormat',inputFormat);


duration = milliseconds(c(end)-c(1))
ms_pro_message = duration/numel(c)
[maximum_diff, pos] = max(diff(a))
[maximum_ms, pos] = max(milliseconds(diff(c)))
plot(movmean(milliseconds(diff(c)),1000))
figure;

plot(diff(a))



% a = str2double(table2array(Data{1}(:,1)));
% b = table2array(Data{1}(:,2));
% inputFormat = 'yyyy.MM.dd HH:mm:ss.SSS';
% c = datetime(b,'InputFormat',inputFormat);
% 
% 
% duration = milliseconds(c(end)-c(1))
% ms_pro_message = duration/numel(c)
% [maximum, pos] = max(diff(a))

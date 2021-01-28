function plotxAndy()
fid = fopen('TCPtestConv.txt','r');

i = 1;
while true
    a(i) = str2double(fgetl(fid));
    c(i) = str2double(fgetl(fid));

    if isnan(c(i))
        break;
    end
    if a(i) == 65535
        if c(i) ~= 65535
            error("fehler")
        end
        i = i-1;        
    end
    i = i+1;
end
figure();
hold on;
plot(a(1:end-1));

% figure();
% plot(b(1:end-1));

% figure();
plot(c(1:end-1));


fclose(fid);

end
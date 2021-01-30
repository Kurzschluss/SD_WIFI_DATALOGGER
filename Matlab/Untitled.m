pause on 
tcp = tcpclient("192.168.137.14",23,"Timeout",400);
configureTerminator(tcp,13)

dtnow = now;
dtStart = dtnow+minutes(0)+seconds(10);
dtEnd = dtStart+minutes(0)+seconds(10);
dtNextMeeting = dtStart+minutes(0)+seconds(120);

while true
    a = convertCharsToStrings(char(readline(tcp)));
    if a == "start of transmission"
        break;
    end
end
flush(tcp);
writeline(tcp, "flush");
a = datestr(dtnow,'yyyymmddHHMMSS')
writeline(tcp, a);
a = datestr(dtStart,'yyyymmddHHMMSS');
writeline(tcp, a);
a = datestr(dtEnd,'yyyymmddHHMMSS');
writeline(tcp, a);
a = datestr(dtNextMeeting,'yyyymmddHHMMSS');
writeline(tcp, a);

LinesReceived = 0;
data = [];
% dataReceived1 = char(readline(tcp));
while 1
    if tcp.NumBytesAvailable > 0
        dataReceived = convertCharsToStrings(char(readline(tcp)));
        dataReceived = erase(dataReceived,char(newline));
        if dataReceived == "end of transmission"
            break
        end
        data = [data dataReceived];
        LinesReceived = LinesReceived + 1
    end 
end
fid = fopen('ny.txt','wt');
fprintf(fid,'%s\n', data{:});
fclose(fid);
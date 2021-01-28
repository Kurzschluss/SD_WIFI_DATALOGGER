clear;

t = tcpclient("192.168.137.14",23,"Timeout",4000);
configureTerminator(t,13)
 
LinesReceived = 0;
data = [];
% dataReceived1 = char(readline(t));
while 1
    if t.NumBytesAvailable > 0
        dataReceived = convertCharsToStrings(char(readline(t)));
        dataReceived = erase(dataReceived,char(newline));
        if dataReceived == "end of transmission"
            break;
        end
        data = [data dataReceived];
        LinesReceived = LinesReceived + 1
    end 
end
fid = fopen('ny.txt','wt');
fprintf(fid, 'Happy\nNew\nYear');
fclose(fid);

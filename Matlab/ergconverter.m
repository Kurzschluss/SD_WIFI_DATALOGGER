function ergconverter()
fid = fopen('TCPtest.txt','r');
fid2 = fopen('TCPtestConv.txt','w');

while true
    a = str2double(fgetl(fid));
    b = str2double(fgetl(fid));
    if isnan(a)
        break;
    end
    if isnan(b)
        break;
    end

    a = de2bi(a,8,'left-msb');
    b = de2bi(b,8,'left-msb');
    c = [b,a];
    c = bi2de(c,'left-msb');
    fprintf(fid2, '%u\n', c);
end

fclose(fid);
fclose(fid2);
end
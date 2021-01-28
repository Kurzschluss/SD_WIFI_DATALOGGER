clear;
addpath('src')
ButtonHandle = uicontrol('Style', 'PushButton', ...
                         'String', 'Stop loop', ...
                         'Callback', 'delete(gcbf)');
t = tcpclient("192.168.137.36",8888);
configureTerminator(t,35)
% udpr = dsp.UDPReceiver('LocalIPPort',4211);
% udps = dsp.UDPSender('RemoteIPPort',4210, 'RemoteIPAddress','192.168.137.116');
% 
% setup(udpr); 
bytesReceived = 0;
% 
% dataSent = uint8('M');
% udps(dataSent);


Data = {};
TCPtestdata = [];

iterator = 1;
drawnow
% format = 'yyyy.mm.dd HH:MM:SS.FFF'
format = 'MM:SS.FFF';
fid = fopen('TCPtest.txt','wt');
while 1
    if t.NumBytesAvailable > 0
        dataReceived = readline(t);
        message = dataReceived;
%         Data = MessageHandler(Data, dataReceived);
        TCPtestdata = [TCPtestdata ;dataReceived datestr(now,format)];
%         fprintf(fid, dataReceived);
%         fprintf(fid, '\n');

%         number = str2double(extractBefore(message, 2));
%         message = extractAfter(message, 1);
% 
%         if numel(Data) < number
%             Message = [];
%             Date_saved = [];
%             T = table(Message, Date_saved);
%             Data{number} = T;
%         else
%             T = {message, datestr(now,'yyyy.mm.dd HH:MM:ss.FFF')};
%             Data{number} = [Data{number}; T];
%         end



    end
    if ~ishandle(ButtonHandle)
        disp('Loop stopped by user');
        break;
    end
end

fclose(fid);
fprintf('bytesReceived:     %d\n', bytesReceived);
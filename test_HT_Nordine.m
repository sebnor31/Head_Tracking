clc
clear all;
close all;

data1 = [204 204 3 232 1 0];

s_wr = serial('COM78','BaudRate',921600,'DataBits',8, 'Parity', 'none', 'StopBits', 1, 'InputBufferSize', 1);
connected = 0;
fopen (s_wr);
while (connected ~= 1)
    
    fwrite(s_wr, data1);
    a = fread(s_wr);
    if (~isempty(a))
        msgbox('TDS Connected');
        fclose(s_wr);
        delete(s_wr);
        %clear s_wr;
        connected = 1;
    else
    disp('Power reset the headset');   
    end
    pause (0.01);
    
end

% getting the data

%count1 = 0;            % number of usb buffer read
x = 1;
%y = 1;
%c = [];
%if (cnct == 1)
byte_read = 28;

% load headtrack
% datasample_previous = zeros(1,17);
% mouse = [0,0];
% mouse_trajectory = zeros(len,2);
% datasamples = zeros(len,17);
% 
% import java.awt.Robot;
% mouse_cursor = Robot;

data3 = [170 170 15 255 15 255];
s_wr = serial('COM78','BaudRate',921600,'DataBits',8, 'Parity', 'none', 'StopBits', 1, 'InputBufferSize',byte_read);
fopen(s_wr);


for d = 1:10

        
        fwrite(s_wr, data3);
        a_data = fread (s_wr);
       
        
        if ((length(a_data)>=byte_read) && (a_data(1) == 0))
            b = a_data;
                                                              
        else
            
            if (x == 1)
                b = zeros(byte_read,1);
            else                
                b = b;
            end
        end
        
%     if(x >= 100)
%         flushoutput(s_wr); 
%         flushinput(s_wr);
%         x = 1;
%     else
%         x = x+1;
%     end 
        
          j = 1;
   
        % accelerometer
        sensorx5 = b(j+0+4)*2^8 + b(j+1+4);
        sensory5 = b(j+2+4)*2^8 + b(j+3+4);
        sensorz5 = b(j+4+4)*2^8 + b(j+5+4);
        
        % gyroscope
        sensorx6 = b(j+12+4)*2^8 + b(j+13+4);
        sensory6 = b(j+14+4)*2^8 + b(j+15+4);
        sensorz6 = b(j+16+4)*2^8 + b(j+17+4);
        
%         sensorx5 = b(j+24+4)*2^8 + b(j+25+4);
%         sensory5 = b(j+26+4)*2^8 + b(j+27+4);
%         sensorz5 = b(j+28+4)*2^8 + b(j+29+4);
%         
%         sensorx6 = b(j+30+4)*2^8 + b(j+31+4);
%         sensory6 = b(j+32+4)*2^8 + b(j+33+4);
%         sensorz6 = b(j+34+4)*2^8 + b(j+35+4);


    datasample = [sensorx5 sensory5 sensorz5 sensorx6 sensory6 sensorz6]
%     disp(datasample);
%     [datasample,mouse] = mTDS_ProportionalHeadControl(datasample, datasample_previous, mouse, Rotation, GyroOffset, Rotation_pitch_roll,1);
%     datasample_previous = datasample;
%disp(mouse);
    %     datasamples(j,:) = datasample;
%     mouse_trajectory(j,:) = mouse;



 %MATLAB_MouseMovementAnimator('string', 76, mouse)



% mouse_cursor.mouseMove(mouse(1)*25, mouse(2)*30);         % sahadat





end

fclose(s_wr);
delete(s_wr);



function colormap(L)

% color mapping
[rows,cols] = size(L);
ColorPic1Map1(1:rows, 1:cols, 1) = 0; 
ColorPic1Map1(1:rows, 1:cols, 2) = L(1:rows, 1:cols);
ColorPic1Map1(1:rows, 1:cols, 3) = 0;
ColorPic1Map1 = uint8(ColorPic1Map1);
imwrite(ColorPic1Map1,'colormapGrayIntensity.png','png');
% imshow(ColorPic1Map1);

Lp(:,:,2) = double(L(:,:))/255;
Lp(:,:,1) = 0.3333;
Lp(:,:,3) = 1;
Lp = hsv2rgb(Lp);
% imshow(Lp);
imwrite(Lp,'colormapGraySaturation.png','png');


%%%%%%%%%%%% Rainbow %%%%%%%%%%%%
info(:,:) = double(L(:,:));
rainbow(:,:,1) = (255-info(:,:)) ./255 .* 240 ./360 ;
rainbow(:,:,2) = 1;
rainbow(:,:,3) = 1;
rainbow = hsv2rgb(rainbow);

% imshow(rainbow);
imwrite(rainbow,'colormapGrayrainbow.png','png');


blueRed = zeros(rows,cols,3);
for i = 1:rows
    for j = 1:cols
        
        if info(i,j) <= 128
            
            blueRed(i,j,1) = info(i,j)./128.*255;
            blueRed(i,j,2) = info(i,j)./128.*255;
            blueRed(i,j,3) = 255;
        else
            blueRed(i,j,1) = 255;
            blueRed(i,j,2) =(255 - info(i,j))./128.*255;
            blueRed(i,j,3) =(255 - info(i,j))./128.*255;
           
        end
    end
end
    blueRed = uint8(blueRed);
    imwrite(blueRed,'colormapGrayBluered.png','png');
    
%%%%%%%%%%%% Two-color interpolation %%%%%%%%%%%%
for i = 1:rows
    for j = 1:cols
        twoColor(i,j,1) = info(i,j);
        twoColor(i,j,2) = 255;
        twoColor(i,j,3) = 0;
    end
end
twoColor = uint8(twoColor);
% imshow(twoColor);
imwrite(twoColor, 'colormapGray2color.png', 'png');


%%%%%%%%%%%% Heated object color scale  %%%%%%%%%%%%

heat = zeros(rows, cols, 3);

for i = 1:rows
    for j = 1:cols
        if info(i, j) <= 85
            heat(i, j, 1) = info(i, j)./85 .* 255;
            heat(i, j, 2) = 0;
            heat(i, j, 3) = 0;

            %heat(i, j, 2) = 0;
        elseif info(i, j) >85 && info(i, j) <= 170
            heat(i, j, 1) = 255; 
            heat(i, j, 2) = (info(i, j)-85)./85 .* 255;
            heat(i, j, 3) = 0;
        else 
            heat(i, j, 1) = 255;
            heat(i, j, 2) = 255;
            heat(i, j ,3) = (info(i, j) - 170)./85 .* 255;
        end
    end
end
heat = uint8(heat);
% imshow(heat);
imwrite(heat, 'colormapGrayheat.png', 'png');


%%%%%%%%%%%% Color scale with contours %%%%%%%%%%%%%%%



colcon(:,:,1) = (255-info(:,:)) ./255 .* 240 ./360 ;
colcon(:,:,2) = 1;
colcon(:,:,3) = 1;

for i = 1:rows;
    for j = 1:cols;
       if (colcon(i,j,1) == 40/360) ||(colcon(i,j,1) == 80/360)||(colcon(i,j,1) == 120/360)||(colcon(i,j,1) == 160/360)||(colcon(i,j,1) == 200/360) ||(colcon(i,j,1) == 240/360) ||(colcon(i,j,1) == 280/360)||(colcon(i,j,1) == 320/360)
            colcon(i:i+4,j,2) = 0;
       end
    end
end

colcon = hsv2rgb(colcon);
% imshow(colcon);
imwrite(colcon,'colormapGraycontour.png','png');


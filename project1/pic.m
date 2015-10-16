function pic(image)

%%%%%%%%%%%%%Formula1 L=sqrt((R^2+G^2+B^2)/3 )%%%%%%%%%%%%%%%%
%PIC1
picture = imread(image);
picture = double(picture);
R = picture(:,:,1);
G = picture(:,:,2);
B = picture(:,:,3);
Lpic(:,:) = ((R.^2 + G.^2 + B.^2)./3).^0.5;
Lpic = uint8(Lpic);
imwrite(Lpic,'image_gray_formula1.png','png');

%%%%%%%%%%%%Formula2 L= 0.30*R + 0.59*G + 0.11*B%%%%%%%%%%%%%%%%
L(:,:) = 0.30*R+0.59*G+0.11*B;
L = uint8(L);
imwrite(L,'image_gray_formula2.png','png');

% % color mapping
% [rows,cols] = size(L);
% ColorPic1Map1(1:rows, 1:cols, 1) = 0; 
% ColorPic1Map1(1:rows, 1:cols, 2) = L(1:rows, 1:cols);
% ColorPic1Map1(1:rows, 1:cols, 3) = 0;
% ColorPic1Map1 = uint8(ColorPic1Map1);
% imwrite(ColorPic1Map1,'image_color_formula1.png','png');
% % imshow(ColorPic1Map1);
% 
% Lp(:,:,2) = double(L(:,:))/255;
% Lp(:,:,1) = 0.3333;
% Lp(:,:,3) = 1;
% Lp = hsv2rgb(Lp);
% % imshow(Lp);
% imwrite(Lp,'image_color_map1.png','png');
% 
% 
% %%%%%%%%%%%% Rainbow %%%%%%%%%%%%
% info(:,:) = double(L(:,:));
% rainbow(:,:,1) = (255-info(:,:)) ./255 .* 240 ./360 ;
% rainbow(:,:,2) = 1;
% rainbow(:,:,3) = 1;
% rainbow = hsv2rgb(rainbow);
% 
% % imshow(rainbow);
% imwrite(rainbow,'image_rainbow.png','png');
% 
% 
% blueRed = zeros(rows,cols,3);
% for i = 1:rows
%     for j = 1:cols
%         
%         if info(i,j) <= 128
%             
%             blueRed(i,j,1) = info(i,j)./128.*255;
%             blueRed(i,j,2) = info(i,j)./128.*255;
%             blueRed(i,j,3) = 255;
%         else
%             blueRed(i,j,1) = 255;
%             blueRed(i,j,2) =(255 - info(i,j))./128.*255;
%             blueRed(i,j,3) =(255 - info(i,j))./128.*255;
%            
%         end
%     end
% end
%     blueRed = uint8(blueRed);
%     imwrite(blueRed,'image_blueRed.png','png');
%     
% %%%%%%%%%%%% Two-color interpolation %%%%%%%%%%%%
% for i = 1:rows
%     for j = 1:cols
%         twoColor(i,j,1) = info(i,j);
%         twoColor(i,j,2) = 255;
%         twoColor(i,j,3) = 0;
%     end
% end
% twoColor = uint8(twoColor);
% % imshow(twoColor);
% imwrite(twoColor, 'image_twocolor.png', 'png');
% 
% 
% %%%%%%%%%%%% Heated object color scale  %%%%%%%%%%%%
% 
% heat = zeros(rows, cols, 3);
% 
% for i = 1:rows
%     for j = 1:cols
%         if info(i, j) <= 85
%             heat(i, j, 1) = info(i, j)./85 .* 255;
%             heat(i, j, 2) = 0;
%             heat(i, j, 3) = 0;
% 
%             %heat(i, j, 2) = 0;
%         elseif info(i, j) >85 && info(i, j) <= 170
%             heat(i, j, 1) = 255; 
%             heat(i, j, 2) = (info(i, j)-85)./85 .* 255;
%             heat(i, j, 3) = 0;
%         else 
%             heat(i, j, 1) = 255;
%             heat(i, j, 2) = 255;
%             heat(i, j ,3) = (info(i, j) - 170)./85 .* 255;
%         end
%     end
% end
% heat = uint8(heat);
% % imshow(heat);
% imwrite(heat, 'image_heat.png', 'png');
% 
% 
% %%%%%%%%%%%% Color scale with contours %%%%%%%%%%%%%%%
% 
% 
% 
% colcon(:,:,1) = (255-info(:,:)) ./255 .* 240 ./360 ;
% colcon(:,:,2) = 1;
% colcon(:,:,3) = 1;
% 
% for i = 1:rows;
%     for j = 1:cols;
%        if (colcon(i,j,1) == 40/360) ||(colcon(i,j,1) == 80/360)||(colcon(i,j,1) == 120/360)||(colcon(i,j,1) == 160/360)||(colcon(i,j,1) == 200/360) ||(colcon(i,j,1) == 240/360) ||(colcon(i,j,1) == 280/360)||(colcon(i,j,1) == 320/360)
%             colcon(i:i+4,j,2) = 0;
%        end
%     end
% end
% 
% colcon = hsv2rgb(colcon);
% % imshow(colcon);
% imwrite(colcon,'colcon_contour.png','png');
colormap(L);
%%%%%%%%%%%%% Red %%%%%%%%%%%%%
picred = uint8(picture(:,:,1));
colormapred(picred);
% figure(1);
% imshow(picred);

%%%%%%%%%%%%% Green %%%%%%%%%%%%%
picgreen = uint8(picture(:,:,2));
% figure(2);
% imshow(picgreen);
colormapgreen(picgreen);
%%%%%%%%%%%%% Green %%%%%%%%%%%%%
picblue = uint8(picture(:,:,3));
% figure(3);
% imshow(picblue);
colormapblue(picblue);
end


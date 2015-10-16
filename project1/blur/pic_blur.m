function pic_blur(image)
%%%%%%%%%%%%%Formula1 L=sqrt((R^2+G^2+B^2)/3 )%%%%%%%%%%%%%%%%
%PIC1


picture = imread(image);
picture = double(picture);
R = picture(:,:,1);
G = picture(:,:,2);
B = picture(:,:,3);
Lpic(:,:) = ((R.^2 + G.^2 + B.^2)./3).^0.5;
Lpic = uint8(Lpic);
imwrite(Lpic,'image_blur_intensity_formula1.png','png');

%%%%%%%%%%%%Formula2 L= 0.30*R + 0.59*G + 0.11*B%%%%%%%%%%%%%%%%
L(:,:) = 0.30*R+0.59*G+0.11*B;
L = uint8(L);
imwrite(L,'image_blur_intensity_formula2.png','png');

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

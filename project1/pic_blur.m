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
imwrite(Lpic,'blur/image_blur_intensity_formula1.png','png');

%%%%%%%%%%%%Formula2 L= 0.30*R + 0.59*G + 0.11*B%%%%%%%%%%%%%%%%
L(:,:) = 0.30*R+0.59*G+0.11*B;
L = uint8(L);
imwrite(L,'blur/image_blur_intensity_formula2.png','png');
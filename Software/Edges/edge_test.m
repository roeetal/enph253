im = imread('ot.PNG');
gray = rgb2gray(im);
T = imcrop(gray,[0 600 1334 100]);
edt = edge(T, 'prewitt');
imshow(edt);
figure();
J = imcrop(gray,[0 400 1334 100]);
ed = edge(J, 'prewitt');
imshow(ed);
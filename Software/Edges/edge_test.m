% https://pdfs.semanticscholar.org/253b/2d9d1dfacf03e50beecc2656740c3005c294.pdf

%% Camera Setup

focalLength    = [309.4362, 344.2161]; % [fx, fy] in pixel units
principalPoint = [318.9034, 257.5352]; % [cx, cy] optical center in pixel coordinates
imageSize      = [480, 640];           % [nrows, mcols]

height = 2.1798;    % mounting height in meters from the ground
pitch  = 14;        % pitch of the camera in degrees

sensor = monoCamera(camIntrinsics, height, 'Pitch', pitch);

%% Load Image

im = imread('img.jpg');
gray = rgb2gray(im);



%%

% Perform Top-Hat Filtering on image
% se = strel('rectangle',[10 20]);
% tophatFiltered = imtophat(gray, se);
% contrastAdjusted = imadjust(tophatFiltered);

% Adaptive threshold
% adaptThresh = adaptthresh(gray, 0.4);
% imshow(adaptThresh);

% T = imcrop(gray,[0 600 1334 100]);
% edt = edge(T, 'prewitt');
% imshow(edt);
% figure();
% J = imcrop(gray,[0 400 1334 100]);
% ed = edge(J, 'prewitt');
% imshow(ed);
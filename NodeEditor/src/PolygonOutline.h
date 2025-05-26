#include <algorithm>
#include <generator>
#include <optional>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <novalis/Polygon.h>
#include <novalis/Texture.h>
#include <novalis/detail/reflection/DataMemberUtil.h>

namespace nv {
    namespace editor {
        cv::Mat pixelDataToMat(const PixelData& pixels) {
            int rowC = pixels.getHeight();
            int colC = pixels.getWidth();

            cv::Mat image(rowC, colC, CV_8UC3);

            for (int y = 0; y < rowC; ++y) {
                for (int x = 0; x < colC; ++x) {
                    auto color = pixels.getColor(x, y);
                    image.at<cv::Vec3b>(y, x) = cv::Vec3b(color.b, color.g, color.r);
                }
            }

            return image;
        }

        inline std::vector<DynamicPolygon> getPolygonOutlines(SDL_Renderer* renderer, const Texture& tex, float worldOffsetX, float worldOffsetY) noexcept {
            auto image = pixelDataToMat(tex.calcPixelData(renderer));

            cv::Mat gray, edges;
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
            cv::GaussianBlur(gray, gray, cv::Size{ 5, 5 }, 1.5);
            cv::Canny(gray, edges, 50, 150);

            std::vector<std::vector<cv::Point>> contours;
			cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			std::vector<DynamicPolygon> polygons;
            for (auto& contour : contours) {
                std::vector<cv::Point> polygon;
                cv::approxPolyDP(contour, polygon, 2.0, true);

                //close the polygon
				auto firstPoint = polygon.front();
                polygon.push_back(std::move(firstPoint));

                std::vector<nv::Point> screenPoints{
                    std::from_range,
                    std::views::transform(polygon, [&](const auto& cvPoint) {
                        return nv::Point{
                            static_cast<float>(cvPoint.x),
                            static_cast<float>(cvPoint.y)
                        }; 
                    })
                };
				
                std::vector worldPoints{
                    std::from_range,
                    std::views::transform(screenPoints, [&](const auto& point) {
                        return nv::Point{
                            point.x + worldOffsetX,
                            point.y + worldOffsetY
                        };
                    })
                };

                auto& poly = polygons.emplace_back(std::move(screenPoints), std::move(worldPoints));
                poly.opacity = 255;
            }
            return polygons;
        }
    }
}
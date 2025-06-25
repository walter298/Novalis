#include <algorithm>
#include <generator>
#include <optional>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <novalis/Polygon.h>
#include <novalis/Texture.h>
#include <novalis/detail/reflection/DataMemberUtil.h>

BOOST_GEOMETRY_REGISTER_POINT_2D(cv::Point, int, boost::geometry::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_RING(std::vector<cv::Point>)
BOOST_GEOMETRY_REGISTER_RING(std::vector<std::vector<cv::Point>>)

BOOST_GEOMETRY_REGISTER_POINT_2D(cv::Point2f, float, boost::geometry::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_RING(std::vector<cv::Point2f>)
BOOST_GEOMETRY_REGISTER_RING(std::vector<std::vector<cv::Point2f>>)

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

            cv::Mat gray, edges, morph;
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
            cv::GaussianBlur(gray, gray, cv::Size{ 5, 5 }, 1.5);
            cv::Canny(gray, edges, 50, 150);
            
            /*use integer points when finding the contours, as using floating points for this step causes 
            opencv to call std::abort()*/
            std::vector<std::vector<cv::Point>> contours;
			cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			std::vector<DynamicPolygon> polygons;
            for (auto& contour : contours) {
                std::vector<cv::Point> polygon;
                cv::approxPolyDP(contour, polygon, 5.0, true);

				auto front = polygon.front();
                polygon.push_back(front);

                if (!boost::geometry::is_valid(polygon)) {
                    continue;
                }

                std::vector<cv::Point2f> polygonF{
                    std::from_range,
                    std::views::transform(polygon, [](const auto& point) {
                        return cv::Point2f{ static_cast<float>(point.x), static_cast<float>(point.y) };
                    })
                };

				std::vector<cv::Point2f> convexPolygon;
				boost::geometry::convex_hull(polygonF, convexPolygon);

                std::vector<nv::Point> screenPoints{
                    std::from_range,
                    std::views::transform(convexPolygon, [&](const auto& cvPoint) {
                        return nv::Point{
                            cvPoint.x,
                            cvPoint.y
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

                DynamicPolygon poly{ std::move(screenPoints), std::move(worldPoints) };
                poly.opacity = 255;
                polygons.push_back(std::move(poly));
            }
            return polygons;
        }
    }
}
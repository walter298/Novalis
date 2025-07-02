#include "PolygonOutline.h"

#include <algorithm>
#include <optional>
#include <mdspan>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

BOOST_GEOMETRY_REGISTER_POINT_2D(cv::Point, int, boost::geometry::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_RING(std::vector<cv::Point>)
BOOST_GEOMETRY_REGISTER_RING(std::vector<std::vector<cv::Point>>)

BOOST_GEOMETRY_REGISTER_POINT_2D(cv::Point2f, float, boost::geometry::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_RING(std::vector<cv::Point2f>)
BOOST_GEOMETRY_REGISTER_RING(std::vector<std::vector<cv::Point2f>>)

class PixelData {
private:
	SDL_Surface* m_surface = nullptr;
public:
	struct Pixel {
		int x = 0;
		int y = 0;
	};
	PixelData(SDL_Renderer* renderer, SDL_Texture* tex, const SDL_FRect& frect) {
		SDL_RenderClear(renderer);
		SDL_RenderTexture(renderer, tex, nullptr, &frect);
		m_surface = SDL_RenderReadPixels(renderer, nullptr);
		if (!m_surface) {
			throw;
		}
	}
	PixelData(const PixelData&) noexcept = delete;
	PixelData(PixelData&&) noexcept = default;
	~PixelData() noexcept {
		SDL_DestroySurface(m_surface);
	}

	void setColor(int x, int y, SDL_Color color) noexcept {
		auto res = SDL_WriteSurfacePixel(m_surface, x, y, color.r, color.g, color.b, color.a);
		assert(res);
	}

	SDL_Color getColor(int x, int y) const noexcept {
		SDL_Color ret;
		bool successfullyReadPixelData = SDL_ReadSurfacePixel(m_surface, x, y, &ret.r, &ret.g, &ret.b, &ret.a);
		assert(successfullyReadPixelData);
		return ret;
	}
	int getWidth() const noexcept {
		return m_surface->w;
	}
	int getHeight() const noexcept {
		return m_surface->h;
	}

	template<typename Func>
	void forEachSubRectangle(int w, int h, Func f) {
		assert(w >= 0 && w <= getWidth() && h >= 0 && h <= getHeight());

		std::vector<Pixel> pixelStorage(static_cast<size_t>(w * h));

		using Subrectangle = std::mdspan<Pixel, std::dextents<int, 2>>;
		Subrectangle rectangle{ pixelStorage.data(), h, w }; //IMPORTANT: rows come before columns

		auto fillRectangle = [&](int x, int y) {
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					rectangle[i, j] = { x + j, y + i };
				}
			}
		};

		for (int y = 0; y + h <= getHeight(); y++) {
			for (int x = 0; x + w <= getWidth(); x++) {
				fillRectangle(x, y);
				f(rectangle);
			}
		}
	}

	template<std::invocable<const SDL_Color&> Pred>
	std::optional<Pixel> find(Pred pred) const noexcept {
		for (int x = 0; x < getWidth(); x++) {
			for (int y = 0; y < getHeight(); y++) {
				auto color = getColor(x, y);
				if (pred(color)) {
					return Pixel{ x, y };
				}
			}
		}
		return std::nullopt;
	}
};

static cv::Mat pixelDataToMat(const PixelData& pixels) {
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


std::vector<nv::DynamicPolygon> nv::editor::getPolygonOutlines(SDL_Renderer* renderer, Texture& tex) noexcept {
	try {
		auto sdlRect = tex.ren.sdlRect();
		PixelData pixelData{ renderer, tex.tex.tex, sdlRect };
		auto image = pixelDataToMat(pixelData);

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

			std::vector screenPoints{
				std::from_range,
				std::views::transform(convexPolygon, [&](const auto& cvPoint) {
					return nv::Point{
						cvPoint.x,
						cvPoint.y
					};
				})
			};
			auto worldPoints = screenPoints;

			DynamicPolygon poly{ std::move(screenPoints), std::move(worldPoints) };
			poly.opacity = 255;
			polygons.push_back(std::move(poly));
		}
		return polygons;
	} catch (std::exception e) {
		return {};
	}
}

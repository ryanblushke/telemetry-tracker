#!/usr/bin/python3

import sys
import os
from urllib import request
from PIL import Image
import os
import math


# root_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))
# if root_path not in sys.path: sys.path.append(root_path)

# Code adopted from GoogleMapDownloader.py by Hayden Eskriett - https://gist.github.com/eskriett/6038468

map_directory = "maps"  # os.path.join(root_path, "processes/navigation/maps")

tile_size = 256

def get_tile_coords(lat, lng, zoom):
    """ Finds the tile coord for a given latitude and longitude, at a given zoom. Returns floats"""
    # Use a left shift to get the power of 2
    # i.e. a zoom level of 2 will have 2^2 = 4 tiles
    numTiles = 1 << zoom

    # Find the x_point given the longitude
    point_x = (tile_size / 2 + lng * tile_size / 360.0) * numTiles / tile_size

    # Convert the latitude to radians and take the sine
    sin_y = math.sin(lat * (math.pi / 180.0))

    # Calulate the y coorindate
    point_y = ((tile_size / 2) + 0.5 * math.log((1 + sin_y) / (1 - sin_y)) * -(
            tile_size / (2 * math.pi))) * numTiles / tile_size

    return point_x, point_y


def save_map_image_by_coords(start_lat, start_lng, width_x, width_y, zoom, name):
    """Saves a map image based on latitude and longitude"""
    x, y = get_tile_coords(start_lat, start_lng, zoom)
    save_map_image(int(x), int(y), width_x, width_y, zoom, name)


def save_map_image(start_x, start_y, width_x, width_y, zoom, name):
    """Downloads and saves a image from google maps. Parameters are in tile coords"""
    # Determine the size of the image
    width, height = 256 * width_x, 256 * width_y

    # Create a new image of the size required
    map_img = Image.new('RGB', (width, height))

    for x in range(0, width_x):
        print(f"Processing row {x}")
        for y in range(0, width_y):
            print(f"Processing column {y}")
            url = 'https://mt.google.com/vt/lyrs=s?x=' + str(start_x + x) + '&y=' + str(start_y + y) + '&z=' + str(zoom)

            current_tile = str(x) + '-' + str(y)
            request.urlretrieve(url, current_tile)

            im = Image.open(current_tile)
            map_img.paste(im, (x * 256, y * 256))

            os.remove(current_tile)
    filename = f"{name},{start_x},{start_y},{width_x},{width_y},{zoom}.png"
    map_img.save(os.path.join(map_directory, filename))


class MapManager:
    def __init__(self, name):
        possible_map_files = os.listdir(map_directory)
        for map_file in possible_map_files:
            if name in map_file:
                self._filename = map_file
                break
        else:
            raise RuntimeError(f"Can't find map named {name}")
        self.tile_start_x, self.tile_start_y, self.tile_width_x, self.tile_width_y, self.zoom = \
            [int(x) for x in self._filename.split('.')[0].split(',')[1:]]

    @property
    def map_filename(self):
        return os.path.join(map_directory, self._filename)

    def get_pixel_from_coords(self, lat, lng):
        x, y = get_tile_coords(lat, lng, self.zoom)
        x = (x - self.tile_start_x) * tile_size
        y = (y - self.tile_start_y) * tile_size
        return int(x), int(y)


if __name__ == "__main__":
    save_map_image_by_coords(52.133, -106.630, 10, 10, 20, "eng_building")
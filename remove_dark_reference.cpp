//
// Created by henry on 18-4-17.
//

#include <gdal_priv.h>

#include "CommonFunc.h"

int main() {
    GDALAllRegister();
    GDALDataset* dataset = (GDALDataset*)GDALOpen("/home/henry/project/data/2018_03_30_15_40_44/raw", GA_ReadOnly);
    GDALDataset* dark_dataset = (GDALDataset*)GDALOpen("/home/henry/project/data/2018_03_30_15_40_44/darkReference", GA_ReadOnly);

    int width = dataset->GetRasterXSize();
    int height = dataset->GetRasterYSize();
    int band_count = dataset->GetRasterCount();

    uint16_t *dark_data = new uint16_t[width * band_count];
    dark_dataset->RasterIO(GF_Read, 0, 0, width, 1, dark_data, width, 1, GDT_UInt16, band_count, nullptr, 0, 0, 0);
    uint16_t *data = new uint16_t[width * band_count];

    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("ENVI");
    GDALDataset* out_dataset = driver->Create("/home/henry/project/data/2018_03_30_15_40_44/raw_remove_dark",
                                              width, height, band_count, GDT_UInt16, nullptr);

    for (int i = 0; i < height; ++i) {
        dataset->RasterIO(GF_Read, 0, i, width, 1, data, width, 1, GDT_UInt16, band_count, nullptr, 0, 0, 0);
        for (int k = 0; k < width * band_count; ++k) {
            data[k] -= dark_data[k];
            if (0 > data[k]) {
                data[k] = 0;
            }
        }
        out_dataset->RasterIO(GF_Write, 0, i, width, 1, data, width, 1, GDT_UInt16, band_count, nullptr, 0, 0, 0);
    }

    ReleaseArray(dark_data);
    ReleaseArray(data);
    GDALClose(out_dataset);
    GDALClose(dark_dataset);
    GDALClose(dataset);
}
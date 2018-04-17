#include <iostream>
#include <fstream>
#include <gdal_priv.h>
#include <boost/filesystem.hpp>
#include <map>

using namespace std;
namespace fs = boost::filesystem;

void transpose_hyperimage(fs::path file);
std::string& trim(std::string &s);

int main() {
    GDALAllRegister();
    string root_path = R"(/media/henry/My Passport/hyperspec-nano/20180328)";
    fs::path root_full_path(root_path);
    fs::directory_iterator end_iter;
    for (fs::directory_iterator dir_itr(root_full_path); dir_itr != end_iter; ++dir_itr) {
        if (fs::is_directory(dir_itr->status())) {
            fs::path image_dir_path = dir_itr->path();
            for (fs::directory_iterator img_dir_iter(image_dir_path); img_dir_iter != end_iter; ++img_dir_iter) {
                if (fs::is_regular_file(img_dir_iter->status())) {
                    if (img_dir_iter->path().filename() == "data" || img_dir_iter->path().filename() == "raw") {
                        transpose_hyperimage(img_dir_iter->path());
                        cout << img_dir_iter->path() << endl;
                    }
                }
            }
        }
    }

    return 0;
}

void transpose_hyperimage(fs::path file) {
    GDALDataset *dataset = (GDALDataset *) GDALOpen(file.string().c_str(), GA_ReadOnly);
    if (nullptr == dataset) {
        return;
    }
    int width = dataset->GetRasterXSize();
    int height = dataset->GetRasterYSize();
    int band_count = dataset->GetRasterCount();
    GDALDataType datatype = dataset->GetRasterBand(1)->GetRasterDataType();
    void *data_buf_out, *line_buf;
    int pixel_size = 0;
    if (datatype == GDT_Int16) {
        data_buf_out = new short[width * height * band_count];
        line_buf = new short[width * band_count];
        pixel_size = sizeof(short);
    } else if (datatype == GDT_UInt16) {
        data_buf_out = new ushort[width * height * band_count];
        line_buf = new ushort[width * band_count];
        pixel_size = sizeof(ushort);
    } else if (datatype == GDT_Float32) {
        data_buf_out = new float[width * height * band_count];
        line_buf = new float[width * band_count];
        pixel_size = sizeof(float);
    }

    for (int row = 0; row < height; ++row) {
        dataset->RasterIO(GF_Read, 0, row, width, 1,
                          line_buf, width, 1, datatype,
                          band_count, nullptr, 0, 0, 0);

        for (int band_idx = 0; band_idx < band_count; ++band_idx) {
            for (int col = 0; col < width; ++col) {
                if (datatype == GDT_Int16) {
                    ((short *) (data_buf_out))[width * height * band_idx + col * height + row]
                            = ((short *) line_buf)[band_idx * width + col];
                } else if (datatype == GDT_UInt16) {
                    ((ushort *) (data_buf_out))[width * height * band_idx + col * height + row]
                            = ((ushort *) line_buf)[band_idx * width + col];
                } else if (datatype == GDT_Float32) {
                    ((float *) (data_buf_out))[width * height * band_idx + col * height + row]
                            = ((float *) line_buf)[band_idx * width + col];
                }
            }
        }
    }

    fs::path new_name = file.parent_path() / fs::path(file.filename().string() + "_T");
    GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("ENVI");
    GDALDataset *dataset_out = driver->Create(new_name.string().c_str(), height, width, band_count, datatype, nullptr);
    dataset_out->RasterIO(GF_Write, 0, 0, height, width,
                          data_buf_out, height, width, datatype,
                          band_count, nullptr, 0, 0, 0);

    if (datatype == GDT_Int16) {
        delete [] ((short *) (data_buf_out));
        delete [] ((short *) (line_buf));
    } else if (datatype == GDT_UInt16) {
        delete [] ((ushort *) (data_buf_out));
        delete [] ((ushort *) (line_buf));
    } else if (datatype == GDT_Float32) {
        delete [] ((float *) (data_buf_out));
        delete [] ((float *) (line_buf));
    }
    data_buf_out = nullptr;
    line_buf = nullptr;
    GDALClose(dataset);
    GDALClose(dataset_out);

    //post process of hdr file
    ifstream src_ifs, dst_ifs;
    src_ifs.open(file.string() + ".hdr");
    dst_ifs.open(new_name.string() + ".hdr");
    char *text_buf = new char[1024];
    map<string, string> properties;
    while (dst_ifs.getline(text_buf, 1024)) {
        string line_content = text_buf;
        if (string::npos != line_content.find_first_of('=')) {
            string key = line_content.substr(0, line_content.find_first_of('=') - 1);
            key = trim(key);
            string value = line_content.substr(line_content.find_first_of('=') + 1, line_content.length());
            value = trim(value);
            properties[key] = value;
        }
    }
    dst_ifs.close();
    ofstream dst_ofs;
    dst_ofs.open(new_name.string() + ".hdr");
    while (src_ifs.getline(text_buf, 1024)) {
        string line_content = text_buf;
        if (string::npos != line_content.find_first_of('=')) {
            string key = line_content.substr(0, line_content.find_first_of('=') - 1);
            string value = line_content.substr(line_content.find_first_of('=') + 1, line_content.length());
            key = trim(key);
            value = trim(value);
            if (0 != properties.count(key)) {
                value = properties[key];
            }
            dst_ofs << key << " = " << value << endl;
        } else {
            dst_ofs << line_content;
        }
    }
    src_ifs.close();
    dst_ofs.close();
    delete [] text_buf;
    text_buf = nullptr;
}

std::string& trim(std::string &s)
{
    if (s.empty())
    {
        return s;
    }

    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}
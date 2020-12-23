/*
 * 5G NR PHY Searcher Measurement Database
 */

#include "consts.h"
#include "log_packet.h"
#include "log_packet_helper.h"

const Fmt NrPhySmdb_Fmt[] = {
    {UINT, "Minor Version", 2},
    {UINT, "Major Version", 2},
    {UINT, "Num Layers", 1},
    {SKIP, NULL, 3},
};

const Fmt NrPhySmdb_Carrier_v2_3[] = {
    {UINT, "Raster Freq", 4},
    {UINT, "Num Cells", 1},
    {UINT_ONES_AS_NA, "Serving Cell Index", 1},
    {UINT_ONES_AS_NA, "Serving Cell PCI", 2},
    {UINT_ONES_AS_NA, "Serving SSB", 1},
    {SKIP, NULL, 3},
    {UINT_ONES_AS_NA, "Serving RX Beam 1", 2},
    {UINT_ONES_AS_NA, "Serving RX Beam 2", 2},
    {UINT_ONES_AS_NA, "Serving Antenna Panel", 2},
    {SKIP, NULL, 2},
};

const Fmt NrPhySmdb_Cell_v2_3[] = {
    {UINT, "PCI", 2},
    {UINT, "PBCH SFN", 2},
    {UINT, "Num Beams", 1},
    {SKIP, NULL, 3},
    {NR_RSRP, "Cell Quality RSRP", 4},
    {NR_RSRQ, "Cell Quality RSRQ", 4},
};

const Fmt NrPhySmdb_Beam_v2_3[] = {
    {UINT, "SSB Index", 1},
    {SKIP, NULL, 3},
    {UINT_ZERO_AS_NA, "RX Beam ID 1", 4},
    {UINT_ZERO_AS_NA, "RX Beam ID 2", 4},
    {BYTE_STREAM_LITTLE_ENDIAN, "Cell Timing", 8},
    {NR_RSRP, "RX Beam RSRP 1", 4},
    {NR_RSRP, "RX Beam RSRP 2", 4},
    {NR_RSRP, "Filtered TX Beam RSRP", 4},
    {NR_RSRQ, "Filtered TX Beam RSRQ", 4},
    {NR_RSRP, "Filtered TX Beam RSRP L3", 4},
    {NR_RSRP, "L2NR Filtered TX Beam RSRP L3", 4},
    {NR_RSRQ, "Filtered TX Beam RSRQ L3", 4},
    {NR_RSRQ, "L2NR Filtered TX Beam RSRQ L3", 4},
};

static int _decode_nr_phy_searcher_measurement_database_payload(
        const char *b, int offset, size_t length, PyObject *result) {
    int start = offset;
    int majver = _search_result_int(result, "Major Version");
    int minver = _search_result_int(result, "Minor Version");
    if (majver != 2 || minver != 3) {
        printf("(MI)Unknown 5G NR PHY Searcher Measurement Database "
                "version: v%d.%d\n", majver, minver);
        return 0;
    }

    PyObject *result_carriers = PyList_New(0);
    int num_layers = _search_result_int(result, "Num Layers");
    for (int i = 0; i < num_layers; ++i) {
        PyObject *result_carrier = PyList_New(0);
        offset += _decode_by_fmt(NrPhySmdb_Carrier_v2_3,
                ARRAY_SIZE(NrPhySmdb_Carrier_v2_3, Fmt),
                b, offset, length, result_carrier);

        PyObject *result_cells = PyList_New(0);
        int num_cells = _search_result_int(result_carrier, "Num Cells");
        for (int j = 0; j < num_cells; ++j) {
            PyObject *result_cell = PyList_New(0);
            offset += _decode_by_fmt(NrPhySmdb_Cell_v2_3,
                    ARRAY_SIZE(NrPhySmdb_Cell_v2_3, Fmt),
                    b, offset, length, result_cell);

            PyObject *result_beams = PyList_New(0);
            int num_beams = _search_result_int(result_cell, "Num Beams");
            for (int k = 0; k < num_beams; ++k) {
                PyObject *result_beam = PyList_New(0);
                offset += _decode_by_fmt(NrPhySmdb_Beam_v2_3,
                        ARRAY_SIZE(NrPhySmdb_Beam_v2_3, Fmt),
                        b, offset, length, result_beam);

                PyObject *t = Py_BuildValue("(sOs)",
                        "Beam", result_beam, "dict");
                PyList_Append(result_beams, t);
                Py_DECREF(t);
                Py_DECREF(result_beam);
            }
            PyObject *t = Py_BuildValue("(sOs)",
                    "Beams", result_beams, "list");
            PyList_Append(result_cell, t);
            Py_DECREF(t);
            Py_DECREF(result_beams);

            t = Py_BuildValue("(sOs)",
                    "Cell", result_cell, "dict");
            PyList_Append(result_cells, t);
            Py_DECREF(t);
            Py_DECREF(result_cell);
        }
        PyObject *t = Py_BuildValue("(sOs)",
                "Cells", result_cells, "list");
        PyList_Append(result_carrier, t);
        Py_DECREF(t);
        Py_DECREF(result_cells);

        t = Py_BuildValue("(sOs)",
                "Carrier", result_carrier, "dict");
        PyList_Append(result_carriers, t);
        Py_DECREF(t);
        Py_DECREF(result_carrier);
    }
    PyObject *t = Py_BuildValue("(sOs)",
            "Carriers", result_carriers, "list");
    PyList_Append(result, t);
    Py_DECREF(t);
    Py_DECREF(result_carriers);

    return offset - start;
}

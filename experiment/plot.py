from kresult import dictFromCSV
import sys
import os
from enum import Enum


class BarColour(Enum):
    Cyan = "cyan"
    Orange = "orange"
    Lime = "lime"
    Red = "red"
    Blue = "blue"


DEFAULT_COLOURS = [
    BarColour.Cyan,
    BarColour.Orange,
    BarColour.Lime,
    BarColour.Red,
    BarColour.Blue,
]


def texFromArbitraryCSV(csvPath: str, colours, removeTime=True, yLabel="Time (s)"):
    results = dictFromCSV(csvPath)
    programNames = []
    plots = {}

    for result in results:
        name = result["Program"]
        programNames.append(name)

        for key, value in result.items():
            if key != "Program":
                if removeTime:
                    assert key[-5:] == " Time"
                    key = key[:-5]

                if key not in plots:
                    plots[key] = []

                plots[key].append(f"({name},{value})")

    assert len(colours) >= len(plots)
    colours = colours[: len(plots)]

    addPlots = ""
    colNames = []
    for i, key in enumerate(plots):
        colNames.append(key)
        colour = colours[i].value
        addPlots += (
            f"\\addplot[style={{{colour},fill={colour},mark=none}}, draw=black]\n"
        )
        addPlots += f"\tcoordinates {{{' '.join(plots[key])}}};\n"

    names = ",".join(list(map(str, programNames)))
    legendParam = ",".join(colNames)

    return f"""\\documentclass[border=10pt]{{standalone}}
\\usepackage{{xcolor}}
\\usepackage{{pgfplots}}
\\usepackage{{tikz}}
\\begin{{document}}
\\begin{{tikzpicture}}
    \\begin{{axis}}[
        width  = 1.5 * \\textwidth,
        height = 8cm,
        major x tick style = transparent,
        % tickwidth=10,
        ybar=0,
        bar width=10pt,
        % ymajorgrids = true,
        ylabel = {{{yLabel}}},
        symbolic x coords={{{names}}},
        xtick = data,
        scaled y ticks = false,
        enlarge x limits=0.05,
        ymin=0,
        legend cell align=left,
        legend style={{
                at={{(0.98,0.88)}},
                anchor=south east,
                % column sep=1ex
        }}
    ]
        {addPlots}
        \legend{{{legendParam}}}
    \end{{axis}}
\end{{tikzpicture}}
\end{{document}}
"""


def generateTex(csvPath: str, removeTime, colours=DEFAULT_COLOURS, ylabel="Time (s)"):
    assert csvPath[-4:] == ".csv"
    texPath = csvPath[:-4]
    texPath = f"{texPath}.tex"

    with open(texPath, "w") as f:
        f.write(texFromArbitraryCSV(csvPath, colours, removeTime, ylabel))

    print(f"Generated {texPath}!")
    command = f"xclip -selection clipboard < {texPath}"
    os.system(command)
    print("Ran the collowing command to copy to clipboard:")
    print(command)

    return texPath


if __name__ == "__main__":
    assert len(sys.argv) == 3

    path = sys.argv[1]
    removeTime = int(sys.argv[2])

    generateTex(path, removeTime)

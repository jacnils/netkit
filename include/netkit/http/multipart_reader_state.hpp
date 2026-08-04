#pragma once

namespace netkit::http::utility {
    enum class multipart_state {
        boundary,
        headers,
        data,
        finished
    };
}
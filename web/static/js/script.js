// Get interface list
$(document).ready(function() {

});

var time_window_len;
var elapsed_time = 0;
var timer;
var current_query_para;

$("#updateInterface").click(function () {
    var agent_addr = document.forms["topkQueryPlan"]["agentAddress"].value;
    var htmlobj = $.ajax({ url: "/getiflist", data: {agentAddress: agent_addr}, async: false});
    var iflist = JSON.parse(htmlobj.responseText);
    $("#interfaceName").children().remove();
    for (var interface_info of iflist) {
        $("#interfaceName").append("<option>" + interface_info[0] + "</option>");
    }
});

// Post capture parameters
$("#topkStart").click(function() {
    var ifname = document.forms["topkQueryPlan"]["interfaceName"].value;
    var pktCount = -1;
    var k = document.forms["topkQueryPlan"]["k"].value;
    time_window_len = document.forms["topkQueryPlan"]["timeWindowLen"].value;
    current_query_para = {ifname: ifname, pktCount: pktCount, k: k};
    $.ajax({
        type: "GET", 
        url: "/run_capture", 
        data: current_query_para, 
        success: function (result) {
            elapsed_time = 0;
            $("#result").removeAttr("hidden");
            $("#capture-progress").attr("style", "width: 0%");
            $("#result-table tbody").children().remove();
            timer = setInterval(check_progress, 1000);
            console.log(result);
        }
    });
    return false;
});

function check_progress() {
    elapsed_time += 1;
    if (elapsed_time < time_window_len) {
        $("#capture-progress").attr("style", "width: " + (elapsed_time / time_window_len) * 100 + "%");
    }
    else {
        clearInterval(timer);
        $.ajax({
            type: "GET", 
            url: "/stop_capture", 
            success: function(result) {
                clearInterval(timer);
                if (result == "stopped") {
                    $("#capture-progress").attr("style", "width: 100%");
                    get_result();
                } else {
                    timer = setInterval(wait_for_stop, 1000);
                }
            }
        });
    }
};

function wait_for_stop() {
    $.ajax({
        type: "GET", 
        url: "/get_progress", 
        success: function(result) {
            if (result == "Done") {
                $("#capture-progress").attr("style", "width: 100%");
                clearInterval(timer);
                get_result();
            }
        }
    });
}

function get_result() {
    $.ajax({
        type: "GET", 
        url: "/get_result", 
        success: function(result) {
            var topk_list = JSON.parse(result);
            console.log(topk_list);
            for (var entry of topk_list) {
                var flow_detail = JSON.parse(entry[1]);
                $("#result-table tbody").append('<tr> <th scope="row">' + entry[0] + '</th> <td>' + flow_detail.type + '</td> <td>' + flow_detail.src_ip + '</td> <td>' + flow_detail.src_port + '</td> <td>' + flow_detail.dst_ip + '</td> <td>' + flow_detail.dst_port + '</td> <td>' + entry[2] + '</td> </tr>');
            }
            $("#result-table").removeAttr("hidden");
        }
    });
}


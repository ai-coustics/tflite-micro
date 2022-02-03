/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/lite/kernels/internal/reference/round.h"

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"

namespace tflite {
namespace ops {
namespace micro {
namespace round {

constexpr int kInputTensor = 0;
constexpr int kOutputTensor = 0;

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  MicroContext* micro_context = GetMicroContext(context);

  TfLiteTensor* input =
      micro_context->AllocateTempInputTensor(node, kInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor* output =
      micro_context->AllocateTempOutputTensor(node, kOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  TF_LITE_ENSURE_TYPES_EQ(context, input->type, kTfLiteFloat32);
  TF_LITE_ENSURE_TYPES_EQ(context, output->type, input->type);
  TF_LITE_ENSURE_EQ(context, output->bytes, input->bytes);
  TF_LITE_ENSURE_EQ(context, output->dims->size, input->dims->size);
  for (int i = 0; i < output->dims->size; ++i) {
    TF_LITE_ENSURE_EQ(context, output->dims->data[i], input->dims->data[i]);
  }

  micro_context->DeallocateTempTfLiteTensor(input);
  micro_context->DeallocateTempTfLiteTensor(output);
  return kTfLiteOk;
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kInputTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

#if HIFI_VFPU && (defined(HIFI4) || defined(HIFI5))
  const int flat_size = MatchingFlatSize(tflite::micro::GetTensorShape(input), tflite::micro::GetTensorShape(output));
  const float* in_data = tflite::micro::GetTensorData<float>(input);
  float* out_data = tflite::micro::GetTensorData<float>(output);

  int err;
  err = xa_nn_elm_round_f32_f32(out_data,
                                in_data,
                                flat_size
                              );
  TF_LITE_ENSURE(context, (err==0) );
#else
  reference_ops::Round(tflite::micro::GetTensorShape(input),
                       tflite::micro::GetTensorData<float>(input),
                       tflite::micro::GetTensorShape(output),
                       tflite::micro::GetTensorData<float>(output));
#endif // HIFI_VFPU && (defined(HIFI4) || defined(HIFI5))

  return kTfLiteOk;
}
}  // namespace round

TfLiteRegistration Register_ROUND() {
  return {/*init=*/nullptr,
          /*free=*/nullptr,
          /*prepare=*/round::Prepare,
          /*invoke=*/round::Eval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

}  // namespace micro
}  // namespace ops
}  // namespace tflite
